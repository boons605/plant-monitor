/*
 * PlantMonitorMQTTManager.c
 *
 *  Created on: 26 Dec 2020
 *      Author: cdromke
 */


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "GlobalSettings.h"
#include "TimeMgmt.h"
#include "PlantMonitorMQTTManager.h"

#define TOPICNAMEPATTERN "/PlantMonitor%02x%02x%02x%02x%02x%02x/Channel"

static const char* AppName = "PlantMonitorMQTT";


typedef enum {
	WaitWifi = 0U,
	WaitMQTTConnect = 1U,
	UpdatingTopics = 2U,
	Done = 3U
} MQTTMgmtState;

typedef struct
{
	uint32_t DataValue;
	uint8_t status;
} ChannelData;

static ChannelData channels[NO_CHANNELS] = {0};

static uint8_t mqttConnected = 0U;
static uint8_t wifiConnected = 0U;
static uint8_t mqttConnectRequest = 0U;

static uint8_t currentChannelUpdating = 0U;

static MQTTMgmtState state = WaitWifi;
static uint32_t stateEntryTime = 0U;
static esp_mqtt_client_handle_t client;

static char topicName[TOPICNAMEMAXLEN] = {0};

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{

    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:

        	/*
            ESP_LOGI(AppName, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
            ESP_LOGI(AppName, "sent publish successful, msg_id=%d", msg_id);

            */
        	mqttConnected = 1U;
        	mqttConnectRequest = 3U;
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(AppName, "MQTT_EVENT_DISCONNECTED");
            mqttConnected = 0U;
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(AppName, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(AppName, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(AppName, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            if (currentChannelUpdating < NO_CHANNELS)
            {
            	channels[currentChannelUpdating].status = 3U;
            }
            currentChannelUpdating++;
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(AppName, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(AppName, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(AppName, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(AppName, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

uint8_t AllChannelsUpdated(void)
{
	uint8_t index = 0U;
	uint8_t retVal = 1U;

	for (index = 0U; index < NO_CHANNELS; index++)
	{
		if (channels[index].status < 3U)
		{
			retVal = 0U;
		}
	}

	return retVal;
}

void ConnectMQTT(void)
{
	mqttConnectRequest = 1U;
}

void UpdateChannel(uint8_t channelIndex, uint32_t channelValue)
{
	if (channelIndex < NO_CHANNELS)
	{
		channels[channelIndex].status = 1U;
		channels[channelIndex].DataValue = channelValue;
	}
}

void UpdateTopics(void)
{
	if (currentChannelUpdating < NO_CHANNELS)
	{
		ChannelData* channel = &channels[currentChannelUpdating];
		if (channel->status == 1U)
		{
			int msgId;
			char channelTopic[TOPICNAMEMAXLEN];
			char channelValue[8];
			sprintf(channelTopic, topicName, currentChannelUpdating);
			sprintf(channelValue, "%d", channel->DataValue);
			ESP_LOGI(AppName, "Puhlishing channel %d", currentChannelUpdating);
			msgId = esp_mqtt_client_publish(client, channelTopic, channelValue, strlen(channelValue), 1, 0);
			ESP_LOGI(AppName, "sent publish successful, msg_id=%d", msgId);

			if (msgId > -1)
			{
				channel->status = 2U;
			}
		}
		else if (channel->status == 0U)
		{
			currentChannelUpdating++;
		}
		else
		{

		}
	}
	else
	{
		ESP_LOGI(AppName, "Done updating topics");
		state = Done;
	}
}

void RunMQTTManager(void)
{
	MQTTMgmtState prevState = state;

	switch (state)
	{
		case WaitWifi:
			if (wifiConnected == 1U)
			{
				state = WaitMQTTConnect;
			}
			break;
		case WaitMQTTConnect:
			if (mqttConnectRequest == 1U)
			{
				esp_mqtt_client_start(client);
				mqttConnectRequest = 2U;
			}
			if (mqttConnectRequest == 3U)
			{
				state = UpdatingTopics;
			}
			break;
		case UpdatingTopics:
			UpdateTopics();
			break;
		default:
			break;
	}

	if (state != prevState)
	{
		stateEntryTime = GetTimestampMs();
	}
}

void MQTTManagerWifiConnected(void)
{
	wifiConnected = 1U;
}

void MQTTManagerWifiDisconnected(void)
{
	wifiConnected = 0U;
}

void InitMQTTManager(uint8_t* uri)
{

	esp_mqtt_client_config_t mqtt_cfg = {
	        .uri = (char*)uri,
	    };

	esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
	esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
	esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
	esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
	esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
	esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

	client = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);

	stateEntryTime = GetTimestampMs();

	uint8_t mac[8];
	esp_efuse_mac_get_default(mac);

	sprintf(topicName, TOPICNAMEPATTERN, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	strcat(topicName, "%d");

}
