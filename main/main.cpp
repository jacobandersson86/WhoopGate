#include <stdio.h>
#include "cmd_app.h"
#include "wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mqtt.h"
#include "led.h"
#include "ota.h"
#include "nutsbolts.h"
#include "json_create.h"
#include "json_parse.h"

#define TAG "main"

extern "C" void app_main(void);

TaskHandle_t eventLogicTaskHandle;

extern const uint32_t WIFI_CONNECTED = BIT1;
extern const uint32_t MQTT_CONNECTED = BIT2;
extern const uint32_t MQTT_DATA = BIT3;
extern const uint32_t LED_COMMAND = BIT4;
extern const uint32_t WIFI_DISCONNECTED = BIT5;
int mqtt_rdy = 0;


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
                wifiScan();
                break;
            case WIFI_DISCONNECTED:
                mqtt_stop_client();
                break;
            case MQTT_CONNECTED:
                mqtt_subscribe_topics();
                mqtt_rdy = 1;
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
    ledInit();
    json_parse_init();
    wifiInit();
    ota_init();
}

void app_main()
{
    initAll();
    uint8_t id[2];
    nutsbolts_get_id(id);
    while(1)
    {
        if(mqtt_rdy)
            mqtt_publish_id();
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }

    
}
