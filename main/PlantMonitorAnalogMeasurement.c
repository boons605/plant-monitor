/*
 * PlantMonitorAnalogMeasurement.c
 *
 *  Created on: 25 Dec 2020
 *      Author: cdromke
 */

#include <stdint.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "GlobalSettings.h"
#include "PlantMonitorAnalogMeasurement.h"


static const adc_channel_t channelList[NO_LOCAL_ADC_CHANNELS] = {ADC_CHANNEL_3, ADC_CHANNEL_4, ADC_CHANNEL_5, ADC_CHANNEL_6, ADC_CHANNEL_7};
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;
static esp_adc_cal_characteristics_t* adc_chars;

uint8_t InitAnalogMeasurement(void)
{
	uint8_t chanIndex = 0U;
	if(unit == ADC_UNIT_1)
	{
		adc1_config_width(ADC_WIDTH_BIT_12);
	}

	for(chanIndex = 0U; chanIndex < NO_LOCAL_ADC_CHANNELS; chanIndex++)
	{
		//Configure ADC
		if(unit == ADC_UNIT_1)
		{
			//adc1_config_width(ADC_WIDTH_BIT_12);
			adc1_config_channel_atten(channelList[chanIndex], atten);
		}
	}

	 adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	 esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

	 return 1U;
}

uint32_t GetChannelMvValue(uint8_t channel)
{
	uint32_t adc_reading = 0;
	uint32_t retVal = 0U;

	if (channel < NO_LOCAL_ADC_CHANNELS)
	{
		 //Multisampling
		for(int i = 0; i < NO_OF_SAMPLES; i++)
		{
			if(unit == ADC_UNIT_1)
			{
				adc_reading += adc1_get_raw((adc1_channel_t)channelList[channel]);
			}
		}
		adc_reading /= NO_OF_SAMPLES;
		//Convert adc_reading to voltage in mV
		retVal =  esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
	}
	return retVal;
}
