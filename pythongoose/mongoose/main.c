#include "mongoose.h"
#include "mgmp.h"

extern void mgmp_call_cb(const char **bfr, size_t *len);
extern bool mgmp_auth(const char *user, const char *pwd);

static void fn(struct mg_connection *c, int ev, void *ev_data)
{
struct mg_http_message *hm = (struct mg_http_message *) ev_data;

  switch(ev) {
  case MG_EV_HTTP_MSG:
    if (mg_match(hm->uri, mg_str("/api/#"), NULL)) {
      char user[17], pwd[17];
      mg_http_creds(hm, user, sizeof(user), pwd, sizeof(pwd));
      if (!mgmp_auth(user, pwd)) {  // All URIs starting with /api/ must be authenticated
        mg_http_reply(c, 401, "WWW-Authenticate: Basic realm=\"this example\"\r\n", "Auth required\n");
        break;
      }
      if (mg_match(hm->uri, mg_str("/api/epname"), NULL)) {
        const char *bfr = hm->body.buf;
        size_t len = hm->body.len;
        mgmp_call_cb(&bfr, &len);
        mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "%.*s", len, bfr);
      } else {
        mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "%s\n", "Not found");
      }
    } else {
//      mg_http_serve_dir(c, hm, (struct mg_http_serve_opts *)&opts);
        mg_http_reply(c, 200, "", "<H1>%s</H1>\n", "OK");
    }
    break;
  }
}

static struct mg_mgr mgr;

void mgmp_init(void) {
  mg_mgr_init(&mgr);        // Mongoose event manager
  mg_log_set(MG_LL_INFO);   // Set log level

  mg_http_listen(&mgr, "http://0.0.0.0:8000", fn, &mgr);
}

void mgmp_poll(int t) {
  mg_mgr_poll(&mgr, t);
}


static void mqtt_fn(struct mg_connection *c, int ev, void *ev_data) {
  struct mgmp_mqttc_data *celf = (struct mgmp_mqttc_data *) c->fn_data;
  switch (ev) {
  case MG_EV_OPEN:
    *(uint64_t *) c->data = mg_millis() + 3000;
    break;
  case MG_EV_POLL:
    if (mg_millis() > *(uint64_t *) c->data && (c->is_connecting || c->is_resolving))
      mg_error(c, "Connect timeout");
    break;
  case MG_EV_MQTT_OPEN:
    if (*(uint8_t *)ev_data != 0)
      break;
    celf->conn_cb(celf->cb_data);
    break;
  case MG_EV_MQTT_MSG: {
    struct mg_mqtt_message *mm = (struct mg_mqtt_message *) ev_data;
    const char *topic = mg_mprintf("%.*s", mm->topic.len, mm->topic.buf);
    celf->pub_cb(celf->cb_data, topic, (const unsigned char *)mm->data.buf, mm->data.len);
    free((char *)topic);
    break;
    }
  case MG_EV_CLOSE:
    celf->c = NULL;  // Mark that we're closed
    break;
  }
}

static void mqttc_timer_fn(void *arg)
{
struct mgmp_mqttc_data *celf = (struct mgmp_mqttc_data *) arg;

  if (celf->c == 0) {
    struct mg_mqtt_opts opts = {
      .user = mg_str(celf->user),
      .pass = mg_str(celf->pwd),
      .topic = mg_str(celf->will_topic),
      .message = mg_str_n(celf->will_msg, celf->will_mlen),
      .qos = celf->will_qos,
      .version = 4,
      .retain = celf->will_retain,
      .clean = true
    };
    celf->c = mg_mqtt_connect(&mgr, celf->broker, &opts, mqtt_fn, celf);
  }
}

void mgmp_mqttc_connect(struct mgmp_mqttc_data *celf)
{
  mg_timer_add(&mgr, 3000, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, mqttc_timer_fn, celf);
}

void mgmp_mqttc_pub(struct mgmp_mqttc_data *celf, const char *topic, const unsigned char *msg, size_t mlen, int qos, bool retain)
{
struct mg_str pubt = mg_str((char *)topic), data = mg_str_n((char *)msg, mlen);
struct mg_mqtt_opts opts;

    memset(&opts, 0, sizeof(opts));
    opts.topic = pubt;
    opts.message = data;
    opts.qos = qos;
    opts.retain = retain;
    mg_mqtt_pub(celf->c, &opts);
}

void mgmp_mqttc_sub(struct mgmp_mqttc_data *celf, const char *topic, int qos)
{
struct mg_str subt = mg_str((char *)topic);
struct mg_mqtt_opts opts;

    memset(&opts, 0, sizeof(opts));
    opts.topic = subt;
    opts.qos = qos;
    mg_mqtt_sub(celf->c, &opts);
}
