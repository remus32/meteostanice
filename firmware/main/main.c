#include "main.h"

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
RTC_SLOW_ATTR ws_bme280_measurement_t ws_bme280_measurements[WS_MEASUREMENT_STORE_SIZE];
RTC_SLOW_ATTR uint8_t ws_bme280_measurement_idx = 0;

void app_main(void) {
  ws_bme280_measurement_idx++;
  
  ESP_ERROR_CHECK(ws_led_init());
  ws_led_set(0);

  ESP_ERROR_CHECK(ws_bme280_init());

  // ws_ulp_start();

  ws_measurement_t measurement;

  ESP_ERROR_CHECK(ws_bme280_measure(&ws_bme280_measurements[ws_bme280_measurement_idx - 1]));
  // ESP_LOGI(LTAG, "temp = %i.%i°C, pres=%uPa, hum = %u%%rH", bme280_measurement.temp / 100, bme280_measurement.temp % 100, bme280_measurement.pres / 256, bme280_measurement.hum / 1024);

  if (ws_bme280_measurement_idx >= WS_MEASUREMENT_SEND_CYCLES) {
    ws_bme280_measurement_idx = 0;

    measurement = (ws_measurement_t){
      .bme = ws_bme280_measurements,
      .bme_mask = 0b11111
    };

    ws_led_set(50);
    ws_wifi_init();
    ESP_ERROR_CHECK(ws_http_send(&measurement));

  }


  ws_led_set(-1);
  esp_wifi_stop();

  ESP_LOGI(LTAG, "Entering deep sleep...");
  esp_deep_sleep(1000 * 1000 * WS_MEASUREMENT_WAKEUP_INTERVAL);
}
