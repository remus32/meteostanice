#include "main.h"

#include "esp_log.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// https://gist.github.com/MakerAsia/37d2659310484bdbba9d38558e2c3cdb
static const char *LTAG = "meteostanice http";

char tx_buffer[128];

/*
 * Resolvne hostname a vytvoří socket na TCP port 80.
 *
 * Vrací fd nebo chybu (pokud rv < 0)
 */
static int ws_http_create_socket(const char *hostname)  {
  // https://github.com/espressif/esp-idf/blob/e2e4cd1a7fb6e73c1904f247b15799dee50a462f/examples/common_components/protocol_examples_common/addr_from_stdin.c
  esp_err_t err;
  struct addrinfo hints, *addr_list;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; // IPv4 nebo IPv6
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  // Provede DNS dotaz
  err = getaddrinfo(hostname, "80", &hints, &addr_list);
  if (err != 0) {
    ESP_LOGE(LTAG, "Getaddrinfo failed: %s", esp_err_to_name(err));
    return ESP_FAIL;
  }

  int sfd = -1;
  // getaddrinfo dá linked list
  for (struct addrinfo *ad = addr_list; ad != NULL; ad = ad->ai_next) {
    sfd = socket(ad->ai_family, ad->ai_socktype, ad->ai_protocol);
    if (sfd < 0) {
      ESP_LOGW(LTAG, "Unable to create socket: %s", esp_err_to_name(sfd));
      continue;
    }

    err = connect(sfd, ad->ai_addr, ad->ai_addrlen);
    if (err != 0) {
      ESP_LOGW(LTAG, "Unable to connnect socket: %s", esp_err_to_name(err));
      close(sfd);
      sfd = -1;
      continue;
    }

    // Nenastala chyba, v sfd je validní file descriptor
    break;
  }
  freeaddrinfo(addr_list);

  if (sfd < 0) {
    ESP_LOGE(LTAG, "Failed to connect to '%s'", hostname);
    return ESP_FAIL;
  }

  return sfd;
}

// Pomocná makra pro zápis do socketu.
//
#define send_helper_check_errror(expr) { err = expr; if (err < 0) goto on_tx_errror; }
static esp_err_t sendf_impl(int sfd, int chars_printed, void* buffer, size_t buffer_size) {
  assert(chars_printed < buffer_size);
  int bytes = send(sfd, buffer, chars_printed, 0);
  if (bytes < 0) {
    return bytes;
  } else {
    assert(bytes == chars_printed);
    return ESP_OK;
  }
}
#define sendf(sfd, ...) send_helper_check_errror(sendf_impl(sfd, snprintf(tx_buffer, sizeof(tx_buffer), __VA_ARGS__), tx_buffer, sizeof(tx_buffer)))
static esp_err_t sendchunkf_impl(int sfd, int chars_printed, void* buffer, size_t buffer_size) {
  esp_err_t err;
  // https://en.wikipedia.org/wiki/Chunked_transfer_encoding

  char helper_buffer[16];
  err = sendf_impl(
    sfd,
    snprintf(helper_buffer, sizeof(helper_buffer), "%x\r\n", chars_printed),
    helper_buffer,
    sizeof(helper_buffer)
  );
  if (err < 0) {
    return err;
  }

  err = sendf_impl(sfd, chars_printed, buffer, buffer_size);
  if (err < 0) {
    return err;
  }

  return sendf_impl(sfd, 2, "\r\n", 3);
}
#define sendchunk(sfd, str, len) send_helper_check_errror(sendchunkf_impl(sfd, len, str, len + 1))
#define sendchunkf(sfd, ...) send_helper_check_errror(sendchunkf_impl(sfd, snprintf(tx_buffer, sizeof(tx_buffer), __VA_ARGS__), tx_buffer, sizeof(tx_buffer)))

esp_err_t ws_http_send(const ws_measurement_t *measurement) {
  esp_err_t err = ESP_OK;
  int sfd = ws_http_create_socket(WS_HTTP_SERVER_NAME);
  if (sfd < 0) {
    return ESP_FAIL;
  }

  sendf(sfd, "POST %s?key=%s HTTP/1.1\r\n", WS_HTTP_PATH, WS_HTTP_KEY);
  sendf(sfd, "Host: %s\r\n", WS_HTTP_SERVER_NAME);
  sendf(sfd, "Transfer-Encoding: chunked\r\nConnection: close\r\n\r\n");

  sendchunkf(sfd, "{\"bme\":[");

  // Před prvním objektem nebudeme posílat čárku
  int send_comma = 0;
  for (int i = 0;i < 32;i++) {
    // Je v masce zaplý itý bit?
    if (measurement->bme_mask & (1 << i)) {
      // Pošleme ité měření
      const ws_bme280_measurement_t *pt = measurement->bme + i;

      sendchunkf(
        sfd,
        "%s{\"temp\":%i,\"pres\":%u,\"hum\":%u}",
        send_comma++ ? "," : "",
        pt->temp,
        pt->pres,
        pt->hum
      );
    }
  }

  sendchunkf(sfd, "]}");
  sendchunk(sfd, "", 0);

  char c;
  recv(sfd, &c, 1, 0);

on_tx_errror:
  close(sfd);
  return err;
}