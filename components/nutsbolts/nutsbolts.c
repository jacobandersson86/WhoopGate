#include "nutsbolts.h"
#include "esp_system.h"
#include "esp_err.h"
#include <string.h>

void nutsbolts_get_id(uint8_t *id)
{
    uint8_t mac[6]; 
    ESP_ERROR_CHECK(esp_read_mac(mac, 0));
    for(int i = 0; i < 2; i ++)
    {
        id[i] = mac[i] ^ mac[i+2] ^ mac[i+4];
    }
}