/*
 * PlantMonitorManager.c
 *
 *  Created on: 25 Dec 2020
 *      Author: cdromke
 */

#include "GlobalSettings.h"
#include "PlantMonitorManager.h"
#include "PlantMonitorAnalogMeasurement.h"
#include "PlantMonitorMQTTManager.h"
#include <string.h>
#include "esp_log.h"
#include "TimeMgmt.h"


typedef enum {
	WaitInit = 0U,
	WaitWifi = 1U,
	WaitMeasurementStabilize = 2U,
	StartMQTT = 3U,
	UpdateMQTT = 5U,
	Done = 6U

} PlantMonitorMgmtState;

static const char* AppName = "PlantMonitorManager";
static PlantMonitorSettings* configSettings = (PlantMonitorSettings*)0;
static uint8_t wifiConnected = 0U;



static PlantMonitorChannel PMChannels[NO_CHANNELS] = {0};
static PlantMonitorMgmtState state = WaitInit;
static uint32_t stateEntryTime = 0U;

void InitPlantMonitor(PlantMonitorSettings* settings)
{
	configSettings = settings;
	InitMQTTManager(configSettings->mqttUrl);
}

uint8_t InitPlantMonitorChannel(uint8_t channel, double voltageDivider)
{
	uint8_t retVal = 0U;
	if (channel < NO_CHANNELS)
	{
		if (voltageDivider >= 1.0)
		{
			PMChannels[channel].adcValueMv = 0;
			PMChannels[channel].voltageDivider = voltageDivider;
			retVal = 1U;
		}
	}

	return retVal;
}

uint8_t PlantMonitorReadyToSleep(void)
{
	uint8_t retVal = 0U;

	if (state == Done)
	{
		retVal = 1U;
	}

	return retVal;
}

void NotifyWifiConnected(void)
{
	if (wifiConnected != 1U)
	{
		ESP_LOGI(AppName, "WiFi connected");
		wifiConnected = 1U;
		MQTTManagerWifiConnected();
	}
}

void NotifyWifiDisconnected(void)
{
	if (wifiConnected != 0U)
	{
		ESP_LOGI(AppName, "WiFi disconnected");
		wifiConnected = 0U;
		MQTTManagerWifiDisconnected();
	}
}

static void UpdateChannelReading(uint8_t channel)
{
	double channelValue = 0.0;
	if (channel < NO_CHANNELS)
	{
		PlantMonitorChannel* pmChan = &PMChannels[channel];

		if (pmChan->voltageDivider > 0.0)
		{
			channelValue = (double)GetChannelMvValue(channel);
			channelValue *= pmChan->voltageDivider;
			pmChan->adcValueMv = (uint32_t)channelValue;
		}
	}
}

static void UpdateAllChannelsToMQTT(void)
{
	uint8_t index;
	for (index = 0U; index < NO_CHANNELS; index++)
	{
		UpdateChannel(index, PMChannels[index].adcValueMv);
	}
}

void RunPlantMonitorManager(void)
{
	uint8_t chanIndex;

	for (chanIndex = 0U; chanIndex < NO_CHANNELS; chanIndex++)
	{
		UpdateChannelReading(chanIndex);
	}

	PlantMonitorMgmtState prevState = state;

	switch (state)
	{
		case WaitInit:
			if (configSettings != (PlantMonitorSettings*)0)
			{
				state = WaitWifi;
			}
			break;
		case WaitWifi:
			if (wifiConnected != 0U)
			{
				state = WaitMeasurementStabilize;
			}
			break;
		case WaitMeasurementStabilize:
			if (GetTimestampMs() > MEASUREMENTSTABILIZE_TIME)
			{
				state = StartMQTT;
			}
			break;
		case StartMQTT:
			ConnectMQTT();
			UpdateAllChannelsToMQTT();
			state = UpdateMQTT;
			break;
		case UpdateMQTT:
			if (AllChannelsUpdated() == 1U)
			{
				state = Done;
			}
			break;
		default:
			break;

	}

	if (prevState != state)
	{
		stateEntryTime = GetTimestampMs();
	}

	RunMQTTManager();

}
