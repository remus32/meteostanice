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

RTC_SLOW_ATTR ws_bme280_measurement_t ws_bme280_measurements[WS_MEASUREMENT_STORE_SIZE];

// Kolik máme v bufferu obsazených pozic?
// RTC_SLOW_ATTR uint8_t ws_bme280_measurement_buffer = 0;
// V jaké fázi jsme?
RTC_SLOW_ATTR uint8_t ws_bme280_measurement_idx = 0;

static void print_chip_info() {
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
    CONFIG_IDF_TARGET,
    chip_info.cores,
    (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
    (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

  printf("silicon revision %d, ", chip_info.revision);

  printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
    (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

  printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

}

void app_main(void) {
  ws_bme280_measurement_idx++;

  print_chip_info();
  ESP_ERROR_CHECK(ws_led_init());
  ws_led_set(200);

  ws_wifi_init();
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
    ESP_ERROR_CHECK(ws_http_send(&measurement));

  }


  ws_led_set(-1);
  esp_wifi_stop();

  ESP_LOGI(LTAG, "Entering deep sleep...");
  esp_deep_sleep(1000 * 1000 * WS_MEASUREMENT_WAKEUP_INTERVAL);


  // for (int i = 10; i >= 0; i--) {
  //     printf("Restarting in %d seconds...\n", i);
  //     vTaskDelay(1000 / portTICK_PERIOD_MS);
  // }
  // printf("Restarting now.\n");
  // fflush(stdout);

  // return;

  // esp_restart();
}
