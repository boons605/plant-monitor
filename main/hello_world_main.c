/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "TimeMgmt.h"
#include "GlobalSettings.h"
#include "PlantMonitorSettings.h"
#include "PlantMonitorManager.h"
#include "PlantMonitorWifiManager.h"
#include "PlantMonitorAnalogMeasurement.h"

const char* AppName = "PlantMonitorMain";

#ifdef CONFIG_IDF_TARGET_ESP32
#define CHIP_NAME "ESP32"
#endif


static PlantMonitorSettings settings = {0};

//Just for fun, no real purpose
static void DumpChipInfo(void)
{
	/* Print chip information */
	    esp_chip_info_t chip_info;
	    esp_chip_info(&chip_info);
	    printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
	            CHIP_NAME,
	            chip_info.cores,
	            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
	            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

	    printf("silicon revision %d, ", chip_info.revision);

	    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
	            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
}

static void GoToSleep(void)
{
	ESP_LOGI(AppName, "Enabling timer wakeup, %ds\n", SLEEPTIME);
	esp_sleep_enable_timer_wakeup(SLEEPTIME * 1000000);

	esp_deep_sleep_start();
}

static void SetupReconfigureInput(void)
{
	gpio_config_t io_conf;
	    //interrupt of rising edge
	    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	    //bit mask of the pins, use GPIO4/5 here
	    io_conf.pin_bit_mask = (1ULL << 12);
	    //set as input mode
	    io_conf.mode = GPIO_MODE_INPUT;
	    //enable pull-up mode
	    io_conf.pull_up_en = 0;
	    io_conf.pull_down_en = 1;
	    gpio_config(&io_conf);
}


void app_main(void)
{
	SetupReconfigureInput();

	InitSettings();
    DumpChipInfo();

    ReadSettings(&settings, gpio_get_level(12));

    InitPlantMonitor(&settings);

    InitWifiManager(&settings);

    InitAnalogMeasurement();

    uint8_t chanIndex;

    for (chanIndex = 0U; chanIndex < NO_CHANNELS; chanIndex++)
    {
    	InitPlantMonitorChannel(chanIndex, 2.0);
    }

    ConnectWifi();

    while (1)
    {
    	if (IsWifiConnected() != 0U)
    	{
    		NotifyWifiConnected();
    	}

    	RunPlantMonitorManager();

    	if ((PlantMonitorReadyToSleep() == 1U) ||
    			(WifiConnectionFailed() == 1U) ||
				(GetTimestampMs() > OPERATION_TIMEOUT))
    	{
    		if (gpio_get_level(12) == 0U)
    		{
    			GoToSleep();
    		}
    	}
    	vTaskDelay(10 / portTICK_PERIOD_MS);
    }

}
