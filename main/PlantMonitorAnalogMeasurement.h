/*
 * PlantMonitorAnalogMeasurement.h
 *
 *  Created on: 25 Dec 2020
 *      Author: cdromke
 */

#ifndef MAIN_PLANTMONITORANALOGMEASUREMENT_H_
#define MAIN_PLANTMONITORANALOGMEASUREMENT_H_

#include <stdint.h>
#include "driver/adc.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

uint8_t InitAnalogMeasurement(void);
uint32_t GetChannelMvValue(uint8_t channel);

#endif /* MAIN_PLANTMONITORANALOGMEASUREMENT_H_ */
