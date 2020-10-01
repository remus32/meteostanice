#include "main.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "driver/gpio.h"

// je zapnuto 1 nebo 0?
#define LED_INVERT -1

static TimerHandle_t blink_timer;
static bool current_state = false;

static void blink_timer_cb(TimerHandle_t _timer) {
  current_state = !current_state;
  gpio_set_level(WS_LED_GPIO, LED_INVERT * current_state);
}

esp_err_t ws_led_init() {
  esp_err_t err;

  // Ledkovej pin do output modu
  err = gpio_set_direction(WS_LED_GPIO, GPIO_MODE_OUTPUT);
  if (err < 0) return err;

  // Zaregistrovat blikací timer
  blink_timer = xTimerCreate(
    "Status LED blinker",
    // defaultni frekvence (prepise se)
    1000000,
    true,
    NULL,
    blink_timer_cb
  );
  if (!blink_timer) return ESP_FAIL;

  return ESP_OK;
};

/*
 * Nastaví stavovou ledku.
 * status < 0 znamená vypnout
 * status = 0 znamená zapnout
 * status > 0 znamená blikat s periodou status milisekund
 */
void ws_led_set(int status) {
  const TickType_t block_time = 1000 / portTICK_PERIOD_MS;

  if (status > 0) {
    TickType_t period = status / portTICK_PERIOD_MS;

    xTimerChangePeriod(blink_timer, period, block_time);
    xTimerReset(blink_timer, block_time);
  } else {
    xTimerStop(blink_timer, block_time);

    current_state = status == 0;
    gpio_set_level(WS_LED_GPIO, LED_INVERT * current_state);
  }
};
