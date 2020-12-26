/*
 * PlantMonitorManager.h
 *
 *  Created on: 25 Dec 2020
 *      Author: cdromke
 */

#ifndef MAIN_PLANTMONITORMANAGER_H_
#define MAIN_PLANTMONITORMANAGER_H_

#include <stdint.h>
#include "PlantMonitorSettings.h"

typedef struct {
	double voltageDivider;
	uint32_t adcValueMv;
} PlantMonitorChannel;


void InitPlantMonitor(PlantMonitorSettings* settings);
uint8_t InitPlantMonitorChannel(uint8_t channel, double voltageDivider);
uint8_t PlantMonitorReadyToSleep(void);
void NotifyWifiConnected(void);
void NotifyWifiDisconnected(void);
void RunPlantMonitorManager(void);

#endif /* MAIN_PLANTMONITORMANAGER_H_ */
