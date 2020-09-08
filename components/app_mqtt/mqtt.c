#include <stdio.h>
#include <string.h>
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "nutsbolts.h"

#define URI CONFIG_MQTT_URI
#define TAG "mqtt"

extern TaskHandle_t eventLogicTaskHandle;
extern const uint32_t MQTT_CONNECTED;
extern const uint32_t MQTT_DATA;
extern const uint32_t LED_COMMAND;
extern const char* my_id;

  esp_mqtt_client_config_t mqttConfig = {
      .uri = URI};
  esp_mqtt_client_handle_t client = NULL;

// [PFDE]
void mqtt_event_handler_cb(esp_mqtt_event_handle_t event);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

// [PFUN]
void mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
  switch (event->event_id)
  {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    xTaskNotify(eventLogicTaskHandle, MQTT_CONNECTED, eSetValueWithOverwrite);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;
  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    xTaskNotify(eventLogicTaskHandle, MQTT_DATA, eSetValueWithOverwrite);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  mqtt_event_handler_cb(event_data);
}

// [FUNC]
void mqtt_start_client(void)
{
    client = esp_mqtt_client_init(&mqttConfig);
    ESP_ERROR_CHECK(client != NULL ? ESP_OK : ESP_FAIL);
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client));
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));    
}

void mqtt_subscribe_topics(void)
{
    char topic[256];
    uint8_t id[2];
    nutsbolts_get_id(id);
    sprintf(topic, "/whoopgate/%02x%02x",id[0],id[1]);
    esp_mqtt_client_subscribe(client, topic, 2);
    esp_mqtt_client_subscribe(client, "/whoopgate/all", 2);
}

void mqtt_publish_id(void)
{
  char topic[256];
  uint8_t id[2];
  nutsbolts_get_id(id);
  sprintf(topic, "/whoopgate/%02x%02x/identity",id[0],id[1]);
}