#include "main.h"

#include "esp_log.h"
#include "esp_http_client.h"

static const char *LTAG = "meteostanice http";
static char post_data_buffer[1024];

static esp_err_t ws_http_event_handler(esp_http_client_event_t *ev) {
  ws_server_response_t *res = ev->user_data;
  ESP_LOGI(LTAG, "event!");
  return ESP_OK;
}

#define ws_http_snprintf_measurement(str, size, m) \
  snprintf( \
    str, size, \
    "{\"tsum\":%i,\"hsum\":%u,\"psum\":%u,\"n_bme\":%i}", \
    m->temp_sum, \
    m->hum_sum, \
    m->pres_sum, \
    m->n_bme \
  )

esp_err_t ws_http_send(const ws_measurement_t *measurement, ws_server_response_t *res) {
  esp_http_client_config_t config = {
    .host = WS_HTTP_HOST,
    .path = WS_HTTP_PATH,
    .query = "key=" WS_HTTP_KEY,
    .method = HTTP_METHOD_POST,
    .event_handler = ws_http_event_handler,
    .user_data = (void*)res
  };
  
  esp_http_client_handle_t client = esp_http_client_init(&config);
  esp_http_client_set_header(client, "Content-Type", "application/json");

  post_data_buffer[0] = '[';
  int len = ws_http_snprintf_measurement(post_data_buffer + 1, sizeof(post_data_buffer) - 2, measurement);
  post_data_buffer[len + 1] = ']';
  esp_http_client_set_post_field(client, post_data_buffer, len + 2);

  esp_err_t err = esp_http_client_perform(client);
  if (err == ESP_OK) {
      ESP_LOGI(LTAG, "HTTP POST Status = %d, content_length = %d",
              esp_http_client_get_status_code(client),
              esp_http_client_get_content_length(client));
  } else {
      ESP_LOGE(LTAG, "HTTP POST request failed: %s", esp_err_to_name(err));
  }

  esp_http_client_cleanup(client);
  return err;
}