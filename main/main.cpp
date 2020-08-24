#include <stdio.h>
#include "cmd_app.h"
#include "wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mqtt.h"
#include "led.h"

#define TAG "main"

extern "C" void app_main(void);

TaskHandle_t eventLogicTaskHandle;

extern const uint32_t WIFI_CONNECTED = BIT1;
extern const uint32_t MQTT_CONNECTED = BIT2;
extern const uint32_t MQTT_DATA = BIT3;
extern const uint32_t LED_COMMAND = BIT4;
const char * my_id = CONFIG_WHOOP_ID;


void mainLogicTask(void *para)
{
    uint32_t command = 0;
    for(;;)
    {
        xTaskNotifyWait(0,0,&command, portMAX_DELAY);
        switch (command)
        {
            case WIFI_CONNECTED:
                ESP_LOGI(TAG, "Wifi connected");
                mqtt_start_client();
                break;
            case MQTT_CONNECTED:
                mqtt_subscribe_topics();
                break;
            case MQTT_DATA:
                // Yay, we have some brand new data, lets do something. 
                break;
            case LED_COMMAND:
                // We have parsed a LED command, lets send to the handler
                // This can be triggered either from the encoder/buttons or mqtt
            default:
                break;
        }
    }

}

void mainInit()
{
    xTaskCreate(mainLogicTask, "mainLogicTask", 1024*4,NULL, 5, &eventLogicTaskHandle);
}

void initAll()
{
    mainInit();
    wifiInit();
    ledInit();
}

void app_main()
{
    initAll();
    printf("WhoopGate!\n");
    //wifiScan();
    
}
