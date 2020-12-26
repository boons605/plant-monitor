/*
 * PlantMonitorWifiManager.c
 *
 *  Created on: 25 Dec 2020
 *      Author: cdromke
 */

#include "PlantMonitorWifiManager.h"
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

static const char *AppName = "PlantMonitorWifi";
static PlantMonitorSettings *configSettings = (PlantMonitorSettings*) 0;
static uint8_t connected = 0U;
static uint8_t connectionFailed = 0U;
static wifi_init_config_t cfg = { 0 };

#define EXAMPLE_ESP_MAXIMUM_RETRY  10


static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
		int32_t event_id, void *event_data) {
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
		connected = 0U;
	} else if (event_base == WIFI_EVENT
			&& event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(AppName, "retry to connect to the AP");
		}
		else
		{
			connectionFailed = 1U;
		}
		ESP_LOGW(AppName, "connect to the AP fail");
		connected = 0U;
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t *event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(AppName, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		connected = 1U;
	}
}

void InitWifiManager(PlantMonitorSettings *settings) {
	configSettings = settings;

	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t wCfg = WIFI_INIT_CONFIG_DEFAULT();

	cfg = wCfg;
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(
			esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
	ESP_ERROR_CHECK(
			esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
}

uint8_t IsWifiConnected(void) {
	return connected;
}

uint8_t WifiConnectionFailed(void)
{
	return connectionFailed;
}

void ConnectWifi(void) {
	wifi_config_t wifi_config = { .sta = {
	/* Setting a password implies station will connect to all security modes including WEP/WPA.
	 * However these modes are deprecated and not advisable to be used. Incase your Access point
	 * doesn't support WPA2, these mode can be enabled by commenting below line */
	.threshold.authmode = WIFI_AUTH_WPA2_PSK,

	.pmf_cfg = { .capable = true, .required = false }, }, };

	memcpy(wifi_config.sta.ssid, configSettings->wifiSSID,
			sizeof(wifi_config.sta.ssid));
	memcpy(wifi_config.sta.password, configSettings->wifiPassword,
			sizeof(wifi_config.sta.password));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
}

void DisconnectWifi(void) {

}
