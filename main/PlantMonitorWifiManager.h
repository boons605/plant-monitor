/*
 * PlantMonitorWifiManager.h
 *
 *  Created on: 25 Dec 2020
 *      Author: cdromke
 */

#ifndef MAIN_PLANTMONITORWIFIMANAGER_H_
#define MAIN_PLANTMONITORWIFIMANAGER_H_
#include <stdint.h>
#include "PlantMonitorSettings.h"

void InitWifiManager(PlantMonitorSettings* settings);
uint8_t IsWifiConnected(void);
void ConnectWifi(void);
void DisconnectWifi(void);
uint8_t WifiConnectionFailed(void);


#endif /* MAIN_PLANTMONITORWIFIMANAGER_H_ */
