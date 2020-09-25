#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "FastLED.h"
#include "esp_log.h"

#define NUM_LEDS CONFIG_WHOOP_N_LED
#define DATA_PIN CONFIG_WHOOP_PIN
#define BRIGHTNESS  20
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
#define INPUT_VOLTAGE 5
#define INPUT_AMPARAGE 500 //mA
CRGB leds[NUM_LEDS];

typedef struct {
  CHSV color;
} fastfade_t;
static const char* TAG = "led";

#define FASTFADE_FPS 30

static void _fastfade_cb(void *param){

  fastfade_t *ff = (fastfade_t *)param;

  ff->color.hue++;

  fill_solid(leds,NUM_LEDS,ff->color);

  FastLED.show();

};

static void fastfade(void *pvParameters){

  fastfade_t ff_t = {
    .color = CHSV(0/*hue*/,255/*sat*/,52/*value*/)
  };

  esp_timer_create_args_t timer_create_args = {
        .callback = _fastfade_cb,
        .arg = (void *) &ff_t,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "fastfade_timer"
    };

  esp_timer_handle_t timer_h;

  esp_timer_create(&timer_create_args, &timer_h);

  esp_timer_start_periodic(timer_h, 1000000L / FASTFADE_FPS );

  // suck- just trying this
  while(1){

      vTaskDelay(1000 / portTICK_PERIOD_MS);
  };

}

void ledInit()
{
    FastLED.addLeds<LED_TYPE, DATA_PIN>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(INPUT_VOLTAGE,INPUT_AMPARAGE);
    CHSV color = CHSV(0, 0, 0);
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();

    //xTaskCreatePinnedToCore(&fastfade, "blinkLeds", 4000, NULL, 5, NULL, 0);
}

extern "C" void ledSetColorHSV(uint8_t hue, uint8_t saturation, uint8_t value)
{
  //TODO Implement something to set all elements to the incomming color
  CHSV color = CHSV(hue, saturation, value);
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
  ESP_LOGI(TAG, "H:%i S:%i, V:%i\n", hue, saturation, value);

}