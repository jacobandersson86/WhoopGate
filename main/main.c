#include <stdio.h>
#include "cmd_app.h"
#include "wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//xSemaphoreHandle_t onConnectionHandler;

void initMain()
{
    //onConnectionHandler = xSemaphoreCreateBinary();
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
