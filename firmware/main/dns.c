#include "main.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/dns.h"

//https://www.nongnu.org/lwip/2_0_x/dns_8h.html#ab5a9dec5b22802f91876c53e99f427ae

static const char *LTAG = "meteostanice dns";

// Struktura předávaná found_cb
typedef struct {
  // TaskHandle volajícího tasku
  TaskHandle_t task;
  ip_addr_t *addr;
  bool finished;
} ws_dns_query_t;

// Callback funkce volaná dns_gethostbyname
static void found_cb(const char *name, const ip_addr_t *addr, void *callback_arg) {
  ws_dns_query_t *query = (ws_dns_query_t *)callback_arg;

  // addr je NULL, pokud nastala chyba
  if (addr) {
    *query->addr = *addr;
  } else {
    // query->addr už není potřeba, použijme ho k přenosu chyby
    query->addr = NULL;
  }

  query->finished = true;

  // Vzbudit volající task
  xTaskNotifyGive(query->task);
}

/*
 * Provede A DNS dotaz.
 *
 * Nativní funkce z lwIP má ošklivé callbackové API,
 * vyrobíme z něj "synchronní" pomocí FreeRTOS synchronizace.
 *
 * Výsledek dotazu se cachuje v addr.
 */
esp_err_t ws_dns_query(const char *hostname, ip_addr_t *addr) {
  // Alokace na stacku je v pořádku, funkce nevrátí dřív než to nebude potřeba
  ws_dns_query_t query = {
    .task = xTaskGetCurrentTaskHandle(),
    .addr = addr,
    .finished = false
  };

  // Funkce z lwIP
  err_t err = dns_gethostbyname(hostname, addr, found_cb, &query);

  if (err == ERR_OK) {
    // addr už obsahovalo výsledek.
    return ESP_OK;
  } else if (err == ERR_INPROGRESS) {
    // Začala síťová operace, čekáme na výsledek
    // Zablokujeme task
    ulTaskNotifyTake(true, WS_DNS_WAIT_TICKS);

    // Pokud není query->finished true, ulTaskNotifyTake timeoutnul
    return query.finished ? (query.addr ? ESP_OK : ESP_FAIL) : ESP_ERR_TIMEOUT;
  } else {
    // Chyba
    ESP_LOGW(LTAG, "DNS query failed for '%s', reason=%i", hostname, err);
    return ESP_FAIL;
  }
}