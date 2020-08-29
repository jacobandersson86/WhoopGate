#include <string.h>
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "ota.h"


#define TAG "ota"
#define URL CONFIG_OTA_URL // our ota location
xSemaphoreHandle ota_semaphore;

extern const uint8_t server_cert_pem_start[] asm("_binary_GlobalSign_pem_start");

static esp_err_t client_event_handler(esp_http_client_event_t *evt)
{
  return ESP_OK;
}

esp_err_t validate_image_header(esp_app_desc_t *incoming_ota_desc)
{
  const esp_partition_t *running_partition = esp_ota_get_running_partition();
  esp_app_desc_t running_partition_description;
  esp_ota_get_partition_description(running_partition, &running_partition_description);

  ESP_LOGI(TAG, "current version is %s\n", running_partition_description.version);
  ESP_LOGI(TAG, "new version is %s\n", incoming_ota_desc->version);

  if (strcmp(running_partition_description.version, incoming_ota_desc->version) == 0)
  {
    ESP_LOGW(TAG, "NEW VERSION IS THE SAME AS CURRENT VERSION. ABORTING");
    return ESP_FAIL;
  }
  return ESP_OK;
}

static void ota_run(void *params)
{
  while (true)
  {
    xSemaphoreTake(ota_semaphore, portMAX_DELAY);
    ESP_LOGI(TAG, "Invoking OTA");

    esp_http_client_config_t clientConfig = {
        .url = URL, 
        .event_handler = client_event_handler,
        .cert_pem = (char *)server_cert_pem_start};

    esp_https_ota_config_t ota_config = {
        .http_config = &clientConfig};

    esp_https_ota_handle_t ota_handle = NULL;

    if (esp_https_ota_begin(&ota_config, &ota_handle) != ESP_OK)
    {
      ESP_LOGE(TAG, "esp_https_ota_begin failed");
      continue;
    }

    esp_app_desc_t incoming_ota_desc;
    if (esp_https_ota_get_img_desc(ota_handle, &incoming_ota_desc) != ESP_OK)
    {
      ESP_LOGE(TAG, "esp_https_ota_get_img_desc failed");
      esp_https_ota_finish(ota_handle);
      continue;
    }

    if (validate_image_header(&incoming_ota_desc) != ESP_OK)
    {
      ESP_LOGE(TAG, "validate_image_header failed");
      esp_https_ota_finish(ota_handle);
      continue;
    }

    while (true)
    {
      esp_err_t ota_result = esp_https_ota_perform(ota_handle);
      if (ota_result != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
        break;
    }

    if (esp_https_ota_finish(ota_handle) != ESP_OK)
    {
      ESP_LOGE(TAG, "esp_https_ota_finish failed");
      continue;
    }
    else
    {
      printf("restarting in 5 seconds\n");
      vTaskDelay(pdMS_TO_TICKS(5000));
      esp_restart();
    }
    ESP_LOGE(TAG, "Failed to update firmware");
  }
}

void ota_init()
{
    ota_semaphore = xSemaphoreCreateBinary();
    xTaskCreate(ota_run, "ota", 1024 * 8, NULL, 2, NULL);
}

void ota_begin_firmare_update()
{
    xSemaphoreGive(ota_semaphore);
}

void ota_print_current_firmware_version()
{
    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    esp_app_desc_t running_partition_description;
    esp_ota_get_partition_description(running_partition, &running_partition_description);
    ESP_LOGI(TAG,"current firmware version is: %s", running_partition_description.version);
}
