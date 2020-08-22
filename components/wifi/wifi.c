// [INCL]
#include "wifi.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "esp_event_legacy.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_err.h"

// [DEFS]
#define MAX_APs 20
#define SSID CONFIG_WIFI_SSID
#define PASSWORD CONFIG_WIFI_PASSWORD
#define TAG "wifi"



// [PFDE]
static esp_err_t event_handler(void * ctx, system_event_t *ev);
static char *getAuthModeName(wifi_auth_mode_t auth_mode);
// [PFUN]
static esp_err_t event_handler(void * ctx, system_event_t *event)
{
    switch(event->event_id)
    {
        case SYSTEM_EVENT_STA_START:
            //wifiScan();
            esp_wifi_connect();
            ESP_LOGI(TAG, "Connecting");
            break;
        
        case SYSTEM_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "Connected");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "Got IP");
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "Disconnected");
            break;
        default:
            break;

    }
    return ESP_OK;
}

static char *getAuthModeName(wifi_auth_mode_t auth_mode)
{
    char *names[] = {"OPEN", "WEP", "WPA PSK", "WPA2 PSK", "WPA WPA2 PSK", "MAX"};
    return names[auth_mode];
}

// [FUNC]
void wifiScan()
{
    wifi_scan_config_t wifi_scan_config =
    {
        .ssid = 0,
        .bssid = 0,
        .channel = 0,
        .show_hidden = true
    };
    esp_wifi_scan_start(&wifi_scan_config, true); // OBS blocking mode!
    
    wifi_ap_record_t wifi_records[MAX_APs];

    uint16_t max_records = MAX_APs;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&max_records, wifi_records));

    printf("Found %d access points:\n", max_records);
    printf("\n");
    printf("               SSID              | Channel   | RSSI  | Auth Mode \n");
    printf("-----------------------------------------------------------------\n");
    for(int i = 0;i < max_records; i++)
    {
        printf("%32s | %7d | %4d | %12s\n", (char *)wifi_records[i].ssid,wifi_records[i].primary,wifi_records[i].rssi,getAuthModeName(wifi_records[i].authmode));
    }
    printf("-----------------------------------------------------------------\n");
}  


void wifiInit()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASSWORD
        }
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    ESP_ERROR_CHECK(esp_wifi_start());
}