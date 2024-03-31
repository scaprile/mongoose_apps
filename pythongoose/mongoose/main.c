#include "mongoose.h"

extern void mgmp_call_cb(const char **bfr, size_t *len);
extern bool mgmp_auth(const char *user, const char *pwd);

static void fn(struct mg_connection *c, int ev, void *ev_data)
{
struct mg_http_message *hm = (struct mg_http_message *) ev_data;

  switch(ev) {
  case MG_EV_HTTP_MSG:
    if (mg_http_match_uri(hm, "/api/#")) {
      char user[17], pwd[17];
      mg_http_creds(hm, user, sizeof(user), pwd, sizeof(pwd));
      if (!mgmp_auth(user, pwd)) {  // All URIs starting with /api/ must be authenticated
        mg_http_reply(c, 401, "WWW-Authenticate: Basic realm=\"this example\"\r\n", "Auth required\n");
        break;
      }
      if (mg_http_match_uri(hm, "/api/epname")) {
        const char *bfr = hm->body.ptr;
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

