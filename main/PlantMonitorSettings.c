/*
 * PlantMonitorSettings.c
 *
 *  Created on: 25 Dec 2020
 *      Author: cdromke
 */

#include "PlantMonitorSettings.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static uint8_t settingsChanged = 0U;

static const char* AppName = "PlantMonitorSettings";

int8_t InitSettings(void)
{
	 esp_err_t err = nvs_flash_init();
	 int8_t retVal = 0;
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		// NVS partition was truncated and needs to be erased
		// Retry nvs_flash_init
		err = nvs_flash_erase();

		if (err != ESP_OK)
		{
			retVal = -1;
		}
		else
		{
			err = nvs_flash_init();
			if (err != ESP_OK)
			{
				retVal = -2;
			}
		}
	}

	return retVal;


}

static void ReadStringFromStdIn(uint8_t* buffer, uint8_t maxLen, char* name)
{
	uint8_t count = 0;
	ESP_LOGI(AppName, "Please enter %s\n", name);
	while (count < maxLen) {
		uint8_t c = fgetc(stdin);
		if (c == '\n') {
			buffer[count] = '\0';
			count = maxLen;
			settingsChanged = 1U;
		} else if (c > 0 && c < 127) {
			buffer[count] = c;
			++count;
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	ESP_LOGI(AppName, "Entered string: %s\n", buffer);
}

static void CheckString(uint8_t* string, uint8_t len, char* name)
{
	if (strlen((char*)string) == 0U)
	{
		ReadStringFromStdIn(string, len, name);
	}
}

static int8_t SaveSettingsToFlash(PlantMonitorSettings* settings)
{
	nvs_handle_t my_handle;
	    esp_err_t err;

	    // Open
	    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
	    if (err != ESP_OK) return -1;


	    err = nvs_set_blob(my_handle, "configuration", settings, sizeof(PlantMonitorSettings));

	    if (err != ESP_OK) return -2;

	    // Commit
	    err = nvs_commit(my_handle);
	    if (err != ESP_OK) return -3;

	    // Close
	    nvs_close(my_handle);
	    return 0;
}

static int8_t ReadSettingsFromFlash(PlantMonitorSettings* settings)
{
		nvs_handle_t my_handle;
	    esp_err_t err;

	    // Open
	    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
	    if (err != ESP_OK) return -1;

	    // Read run time blob
	    size_t required_size = 0;  // value will default to 0, if not set yet in NVS
	    // obtain required memory space to store blob being read from NVS
	    err = nvs_get_blob(my_handle, "configuration", NULL, &required_size);
	    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return -2;
	    if (required_size != sizeof(PlantMonitorSettings)) {
	    	ESP_LOGI(AppName, "Nothing saved yet!\n");
	        return -3;
	    } else {
	        err = nvs_get_blob(my_handle, "configuration", settings, &required_size);
	        if (err != ESP_OK) {

	            return -4;
	        }
	    }

	    // Close
	    nvs_close(my_handle);
	    return 0;
}

uint8_t ReadSettings(PlantMonitorSettings* settings, uint8_t reconfigure)
{
	int8_t stat = 0;
	if (reconfigure == 0U)
	{
		stat = ReadSettingsFromFlash(settings);
	}

	if (stat != 0)
	{
		ESP_LOGW(AppName, "Error while reading from flash: %d", stat);
	}

	CheckString(settings->mqttUrl, sizeof(settings->mqttUrl), "MQTT URL");
	CheckString(settings->mqttUser, sizeof(settings->mqttUser), "MQTT User");
	CheckString(settings->mtqqPass, sizeof(settings->mtqqPass), "MQTT Pass");
	CheckString(settings->wifiSSID, sizeof(settings->wifiSSID), "WiFi SSID");
	CheckString(settings->wifiPassword, sizeof(settings->wifiPassword), "WiFi Pass");

	if (settingsChanged != 0U)
	{
		stat = SaveSettingsToFlash(settings);

		if (stat != 0)
		{
			ESP_LOGW(AppName, "Error while writing to flash: %d", stat);
		}
	}

	return 0U;

}
