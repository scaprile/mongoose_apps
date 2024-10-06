#include "pico/stdlib.h"

#include "mongoose.h"

#include "driver_pico-w.h"

static void mqtt_fn(struct mg_connection *c, int ev, void *ev_data)
{
  if (ev == MG_EV_MQTT_OPEN) {
    struct mg_str pubt = mg_str("embedded/related");
    struct mg_str data = mg_str("Pico-W is up and running !");
    struct mg_mqtt_opts pub_opts = {
      .topic = pubt,
      .message = data,
      .qos = 1,
      .retain = false
    };
    mg_mqtt_pub(c, &pub_opts);
    MG_INFO(("PUBLISHED"));
  }
}

static void mif_fn(struct mg_tcpip_if *ifp, int ev, void *ev_data)
{
  if (ev == MG_TCPIP_EV_ST_CHG && ifp->state == MG_TCPIP_STATE_READY) {
    struct mg_mqtt_opts opts = {.clean = true};
    mg_mqtt_connect(ifp->mgr, "mqtt://broker.hivemq.com:1883", &opts, mqtt_fn, NULL);
    MG_INFO(("Got IP"));
  }
}

int main(void)
{
  stdio_init_all();  

  struct mg_mgr mgr;
  mg_mgr_init(&mgr);

  struct mg_tcpip_driver_pico_w_data driver_data = {
    .ssid = WIFI_SSID,
    .pass = WIFI_PASS
  };
  struct mg_tcpip_if mif = {
      .ip = 0,
      .driver = &mg_tcpip_driver_pico_w,
      .driver_data = &driver_data,
      .recv_queue.size = 8192,
      .fn = mif_fn
  };  
  mg_tcpip_init(&mgr, &mif);

  for (;;) {
    driver_pico_w_poll();
    mg_mgr_poll(&mgr, 0);
  }

  return 0;
}

