// Copyright (c) 2020 Cesanta Software Limited
// All rights reserved
// modified to fit this device purpose by Sergio R. Caprile

static const char *s_url = "https://api.telegram.org/" BOT_TOKEN "/sendMessage";
#define MESSAGE "Oh, someone was able to press that tiny button !"

#include "driver/gpio.h"
#include "mongoose.h"

static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_OPEN) {
    // Connection created. Store connect expiration time in c->data
    *(uint64_t *) c->data = mg_millis() + 1000 /* ms */;
  } else if (ev == MG_EV_POLL) {
    if (mg_millis() > *(uint64_t *) c->data &&
        (c->is_connecting || c->is_resolving)) {
      mg_error(c, "Connect timeout");
    }
  } else if (ev == MG_EV_CONNECT) {
    // Connected to server, tell client connection to use TLS
    struct mg_str host = mg_url_host(s_url);
    struct mg_tls_opts opts = {.ca = mg_unpacked("/ca.pem"),
                               .name = mg_url_host(s_url)};
    mg_tls_init(c, &opts);

    // Send request
    char *buffer = mg_mprintf("{%m:%m,%m:%m,%m:%s}", MG_ESC("chat_id"),
                              MG_ESC(CHAT_ID), MG_ESC("text"), MG_ESC(MESSAGE),
                              MG_ESC("disable_notification"), "false");
    int content_length = strlen(buffer);
    mg_printf(c,
              "%s %s HTTP/1.0\r\n"
              "Host: %.*s\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %d\r\n"
              "\r\n",
              "POST", mg_url_uri(s_url), (int) host.len, host.ptr,
              content_length);
    mg_send(c, buffer, content_length);
    free(buffer);
  } else if (ev == MG_EV_HTTP_MSG) {
    // Response, print it
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    printf("%.*s\n", (int) hm->message.len, hm->message.ptr);
    c->is_draining = 1;        // Tell mongoose to close this connection
    *(bool *) c->fn_data = true;  // Re-enable further calls
  } else if (ev == MG_EV_ERROR) {
    *(bool *) c->fn_data = true;  // Error, enable further calls
  }
}

void app_main(void) {
  gpio_config_t io_conf = {io_conf.intr_type = GPIO_INTR_DISABLE,
                           io_conf.mode = GPIO_MODE_INPUT,
                           io_conf.pin_bit_mask = 1ULL << 39,
                           io_conf.pull_down_en = 0, io_conf.pull_up_en = 0};
  gpio_config(&io_conf);

  // Setup wifi. This function is implemented in wifi.c
  // It blocks until connected to the configured WiFi network
  void wifi_init(const char *ssid, const char *pass);
  wifi_init(WIFI_SSID, WIFI_PASS);
  // Connected to WiFi, now start
  struct mg_mgr mgr;
  mg_log_set(MG_LL_INFO);
  mg_mgr_init(&mgr);
  bool enable = true;
  for (;;) {
    mg_mgr_poll(&mgr, 50);
    if (enable && !gpio_get_level(39)) {
      enable = false;
      mg_http_connect(&mgr, s_url, fn, &enable);
    }
  }
}
