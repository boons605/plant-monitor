/*
 * PlantMonitorSettings.h
 *
 *  Created on: 25 Dec 2020
 *      Author: cdromke
 */

#ifndef MAIN_PLANTMONITORSETTINGS_H_
#define MAIN_PLANTMONITORSETTINGS_H_

#include <stdint.h>

#define URLMAXLENGTH 128U
#define MQTTUSERLENGTH 32U
#define MQTTPASSWORDLENGTH 32U
#define SSIDMAXLENGTH 64U
#define WIFIKEYLENGTH 64U

#define STORAGE_NAMESPACE "storage"

#define SLEEPTIME 10U

typedef struct
{
	uint8_t wifiSSID[SSIDMAXLENGTH];
	uint8_t wifiPassword[WIFIKEYLENGTH];
	uint8_t mqttUrl[URLMAXLENGTH];
	uint8_t mqttUser[MQTTUSERLENGTH];
	uint8_t mtqqPass[MQTTPASSWORDLENGTH];
} PlantMonitorSettings;

uint8_t ReadSettings(PlantMonitorSettings* settings, uint8_t reconfigure);
int8_t InitSettings(void);

#endif /* MAIN_PLANTMONITORSETTINGS_H_ */
