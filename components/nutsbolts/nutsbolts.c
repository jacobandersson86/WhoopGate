#include "nutsbolts.h"
#include "esp_system.h"
#include "esp_err.h"
#include <string.h>

void nutsbolts_get_id(uint8_t *id)
{
    uint8_t mac[6]; 
    ESP_ERROR_CHECK(esp_read_mac(mac, 0));
    printf("Base MAC adress is: ");
    for (int i = 0; i < 6; i++)
    {
        printf("%02x:", mac[i]);
    }
    printf("\n");
    printf("ID is: ");
    for(int i = 0; i < 2; i ++)
    {
        id[i] = mac[i] ^ mac[i+2] ^ mac[i+4];
        printf("%02x:", id[i]);
    }
    printf("\n");
}