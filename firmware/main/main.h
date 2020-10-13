#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_system.h"

#include "lwip/dns.h"

#define WS_WIFI_SSID "stoka"
#define WS_WIFI_PASS "net130755"
// #define WS_WIFI_SSID "ffff"
// #define WS_WIFI_PASS "prasatko"
#define WS_WIFI_MAX_RETRIES 10

#define WS_HTTP_HOST "ws.remus32.cz"
#define WS_HTTP_PATH "/publish.php"
#define WS_HTTP_KEY "supertajneheslo"

// Ktery pin je stavova ledka?
#define WS_LED_GPIO 2

#define WS_MEASUREMENT_STORE_SIZE 16
#define WS_MEASUREMENT_WAKEUP_INTERVAL 58
#define WS_MEASUREMENT_SEND_CYCLES 5

typedef struct {
  int32_t temp;
  uint32_t hum;
  uint32_t pres;
} ws_bme280_measurement_t;

typedef struct {
  int32_t temp_sum;
  uint32_t hum_sum;
  uint32_t pres_sum;
  uint8_t n_bme;
} ws_measurement_t;

typedef struct {
  uint32_t sync_time;
} ws_server_response_t;

void ws_wifi_init();
esp_err_t ws_bme280_init();
int32_t ws_bme280_measure(ws_bme280_measurement_t *measurement);

esp_err_t ws_http_send(const ws_measurement_t *measurement, ws_server_response_t *res);

esp_err_t ws_led_init();
void ws_led_set(int status);

void ws_ulp_start();
