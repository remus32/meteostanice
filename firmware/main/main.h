#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_system.h"

#include "lwip/dns.h"

#define WS_WIFI_SSID "stoka"
#define WS_WIFI_PASS "net130755"
#define WS_WIFI_MAX_RETRIES 10

#define WS_DNS_WAIT_TICKS 10000 / portTICK_RATE_MS

#define WS_HTTP_SERVER_NAME "ws.remus32.cz"
#define WS_HTTP_PATH "/publish"
#define WS_HTTP_KEY "supertajneheslo"

// Ktery pin je stavova ledka?
#define WS_LED_GPIO 2

// #define WS_MEASUREMENT_BME_CYCLES 5

typedef struct {
  int32_t temp;
  uint32_t hum;
  uint32_t pres;
} ws_bme280_measurement_t;

typedef struct {
  ws_bme280_measurement_t bme;
} ws_measurement_t;

// dns.c
esp_err_t ws_dns_query(const char *hostname, ip_addr_t *addr);

void ws_wifi_init();
esp_err_t ws_bme280_init();
int32_t ws_bme280_measure(ws_bme280_measurement_t *measurement);

esp_err_t ws_http_send(ws_measurement_t *measurement);

esp_err_t ws_led_init();
void ws_led_set(int status);
