#include "main.h"

#include <string.h>

#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_wifi.h"

#include "driver/gpio.h"

static const char *LTAG = "meteostanice main";

// Paměť s měřeními
RTC_SLOW_ATTR ws_measurement_t ws_measurements[1];
RTC_SLOW_ATTR uint8_t ws_cycle_counter = 0;

void app_main(void) {
  esp_err_t err;
  ws_cycle_counter++;
  
  if ((err = ws_led_init()) != ESP_OK) {
    ESP_LOGW(LTAG, "led_init failed: %s", esp_err_to_name(err));
  }
  ws_led_set(0);

  // ws_ulp_start();

  if (ws_cycle_counter == 1) {
    memset(&ws_measurements[0], 0, sizeof(ws_measurement_t));
  }

  if ((err = ws_bme280_init()) == ESP_OK) {
    ws_bme280_measurement_t bme_m;
    if ((err = ws_bme280_measure(&bme_m)) == ESP_OK) {
      ws_measurements[0].temp_sum += bme_m.temp;
      ws_measurements[0].hum_sum += bme_m.hum;
      ws_measurements[0].pres_sum += bme_m.pres;
      ws_measurements[0].n_bme += 1;
    } else {
      ESP_LOGW(LTAG, "bme280_measure failed: %s", esp_err_to_name(err));
    }
  } else {
    ESP_LOGW(LTAG, "bme280_init failed: %s", esp_err_to_name(err));
  }

  if (ws_cycle_counter >= WS_MEASUREMENT_SEND_CYCLES) {
    ws_cycle_counter = 0;

    ws_led_set(50);
    ws_wifi_init();
    ws_server_response_t res;
    ESP_ERROR_CHECK(ws_http_send(&ws_measurements[0], &res));
  }


  ws_led_set(-1);
  esp_wifi_stop();

  ESP_LOGI(LTAG, "Entering deep sleep...");
  esp_deep_sleep(1000 * 1000 * WS_MEASUREMENT_WAKEUP_INTERVAL);
}
