/*
 * PlantMonitorMQTTManager.h
 *
 *  Created on: 26 Dec 2020
 *      Author: cdromke
 */

#ifndef MAIN_PLANTMONITORMQTTMANAGER_H_
#define MAIN_PLANTMONITORMQTTMANAGER_H_

#include <stdint.h>

#define TOPICNAMEMAXLEN 64U

void InitMQTTManager(uint8_t* uri);
void RunMQTTManager(void);
void MQTTManagerWifiConnected(void);
void MQTTManagerWifiDisconnected(void);

uint8_t AllChannelsUpdated(void);
void ConnectMQTT(void);
void UpdateChannel(uint8_t channelIndex, uint32_t channelValue);

#endif /* MAIN_PLANTMONITORMQTTMANAGER_H_ */
