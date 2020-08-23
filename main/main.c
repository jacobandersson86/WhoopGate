#include <stdio.h>
#include "cmd_app.h"
#include "wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TAG "main"

TaskHandle_t eventLogicTaskHandle;

const uint32_t WIFI_CONNECTED = BIT1;
const uint32_t MQTT_CONNECTED = BIT2;

void LogicTask(void *para)
{
    uint32_t command = 0;
    for(;;)
    {
        xTaskNotifyWait(0,0,&command, portMAX_DELAY);
        switch (command)
        {
            case WIFI_CONNECTED:
                ESP_LOGI(TAG, "Wifi connected");
                // TODO : Init mqtt client, register event and start client
                break;
            case MQTT_CONNECTED:
                //TODO : subscribe to relevant topics.
                break;
            default:
                break;
        }
    }

}

void initMain()
{
    xTaskCreate(LogicTask, "LogicTask", 1024*4,NULL, 5, &eventLogicTaskHandle);
}

void initAll()
{
    initMain();
    wifiInit();
}

void app_main()
{
    initAll();
    printf("WhoopGate!\n");
    //wifiScan();
    
}
