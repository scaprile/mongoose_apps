// main.c provided
void mgmp_init(void);
void mgmp_poll(int);
void mgmp_setlog(int);
// mqtt
typedef void (*mgmp_mqttc_ccb_t)(void *cb_data);
typedef void (*mgmp_mqttc_pcb_t)(void *cb_data, const char *topic, const unsigned char *data, size_t len);

struct mgmp_mqttc_data {
  mgmp_mqttc_ccb_t conn_cb;
  mgmp_mqttc_pcb_t pub_cb;
  void *cb_data;
  struct mg_connection *c;
  const char *broker;
  const char *user;
  const char *pwd;
  const char *will_topic;
  const char *will_msg;
  size_t will_mlen;
  uint8_t will_qos;
  bool will_retain;
};

void mgmp_mqttc_pub(struct mgmp_mqttc_data *celf, const char *topic, const unsigned char *msg, size_t mlen, int qos, bool retain);
void mgmp_mqttc_sub(struct mgmp_mqttc_data *celf, const char *topic, int qos);
void mgmp_mqttc_connect(struct mgmp_mqttc_data *celf);


// module.c provided
void mgmp_call_cb(const char **bfr, size_t *len);
bool mgmp_auth(const char *user, const char *pwd);

