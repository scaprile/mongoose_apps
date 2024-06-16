#include "py/runtime.h"

#include "mgmp.h"

MP_REGISTER_ROOT_POINTER(mp_obj_t mongoose_auth_cb_obj);    // register callback objects so functions are not deleted
MP_REGISTER_ROOT_POINTER(mp_obj_t mongoose_api_cb_obj);     // by the garbage collector

STATIC mp_obj_t mongoose_init(size_t nargs, const mp_obj_t *args)
{
    MP_STATE_PORT(mongoose_api_cb_obj) = *(args++);
    MP_STATE_PORT(mongoose_auth_cb_obj) = *(args++);
    mgmp_init();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mongoose_init_obj, 2, 2, mongoose_init);

bool mgmp_auth(const char *user, const char *pwd)
{
mp_obj_t obj_u, obj_p;

    if (*user == '\0' || *pwd == '\0')
        return false;
    obj_u = mp_obj_new_str(user, strlen(user));
    obj_p = mp_obj_new_str(pwd, strlen(pwd));
    if ((mp_call_function_2(MP_STATE_PORT(mongoose_auth_cb_obj), obj_u, obj_p)) == mp_const_none)
        return false;
    return true;
}

void mgmp_call_cb(const char **bfr, size_t *len)
{
mp_obj_t obj = mp_obj_new_str(*bfr, *len);

    obj = mp_call_function_1(MP_STATE_PORT(mongoose_api_cb_obj), obj);
    *bfr = mp_obj_str_get_data(obj, len);
}

STATIC mp_obj_t mongoose_poll(mp_obj_t t_obj)
{
    mgmp_poll(mp_obj_get_int(t_obj));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mongoose_poll_obj, mongoose_poll);



typedef struct _mongoose_mqttclient_obj_t {
    mp_obj_base_t base;
    mp_obj_t on_connect_cb;
    mp_obj_t on_publish_cb;
    struct mgmp_mqttc_data celf;    // C-side "self"
} mongoose_mqttclient_obj_t;


static void mqttc_conn_fn(void *fn_data)
{
    mongoose_mqttclient_obj_t *self = (mongoose_mqttclient_obj_t *)fn_data;    
    mp_call_function_0(self->on_connect_cb);
}

static void mqttc_pub_fn(void *fn_data, const char *topic, const unsigned char *data, size_t len)
{
    mongoose_mqttclient_obj_t *self = (mongoose_mqttclient_obj_t *)fn_data;
    mp_obj_t topic_obj = mp_const_none, data_obj = mp_const_none;
    if (topic != NULL)
        topic_obj = mp_obj_new_str(topic, strlen(topic));
    if (len > 0)
        data_obj = mp_obj_new_bytes(data, len);
    mp_call_function_2(self->on_publish_cb, topic_obj, data_obj);
}


STATIC mp_obj_t mongoose_mqttclient_make_new(const mp_obj_type_t *type, size_t nargs, size_t nkw, const mp_obj_t *args) {
    mongoose_mqttclient_obj_t *self = mp_obj_malloc(mongoose_mqttclient_obj_t, type);
    self->on_connect_cb = *(args++);
    self->on_publish_cb = *(args++);
    self->celf.conn_cb = mqttc_conn_fn;
    self->celf.pub_cb = mqttc_pub_fn;
    self->celf.cb_data = self;
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t mongoose_mqttclient_connect(size_t nargs, const mp_obj_t *args)
{
    mongoose_mqttclient_obj_t *self = *(args++);
    mp_obj_t obj;
    self->celf.broker = mp_obj_str_get_str(*(args++));
    obj = *(args++);
    if (obj != mp_const_none)
        self->celf.user = mp_obj_str_get_str(obj);
    obj = *(args++);
    if (obj != mp_const_none)
        self->celf.pwd = mp_obj_str_get_str(obj);
    if (nargs > 4) {
        self->celf.will_topic = mp_obj_str_get_str(*(args++));
        self->celf.will_msg = mp_obj_str_get_data(*(args++), &self->celf.will_mlen);
        self->celf.will_qos = mp_obj_get_int(*(args++));
        self->celf.will_retain = (*(args++) == mp_const_true);
    }
    mgmp_mqttc_connect(&self->celf);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mongoose_mqttclient_connect_obj, 4, 7, mongoose_mqttclient_connect);

STATIC mp_obj_t mongoose_mqttclient_pub(size_t nargs, const mp_obj_t *args)
{
    mongoose_mqttclient_obj_t *self = *(args++);
    size_t mlen;
    const char *topic = mp_obj_str_get_str(*(args++));
    const char *message = mp_obj_str_get_data(*(args++), &mlen);
    int qos = mp_obj_get_int(*(args++));
    bool retain = (*(args++) == mp_const_true);
    mgmp_mqttc_pub(&self->celf, topic, (const unsigned char *)message, mlen, qos, retain);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mongoose_mqttclient_pub_obj, 5, 6, mongoose_mqttclient_pub);

STATIC mp_obj_t mongoose_mqttclient_sub(mp_obj_t self_in, mp_obj_t topic_obj, mp_obj_t qos_obj) {
    mongoose_mqttclient_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *topic = mp_obj_str_get_str(topic_obj);
    int qos = mp_obj_get_int(qos_obj);
    mgmp_mqttc_sub(&self->celf, topic, qos);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mongoose_mqttclient_sub_obj, mongoose_mqttclient_sub);

STATIC const mp_rom_map_elem_t mongoose_mqttclient_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_connect), MP_ROM_PTR(&mongoose_mqttclient_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_pub), MP_ROM_PTR(&mongoose_mqttclient_pub_obj) },
    { MP_ROM_QSTR(MP_QSTR_sub), MP_ROM_PTR(&mongoose_mqttclient_sub_obj) },
};
STATIC MP_DEFINE_CONST_DICT(mongoose_mqttclient_locals_dict, mongoose_mqttclient_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    mongoose_type_mqttclient,
    MP_QSTR_MQTTclient,
    MP_TYPE_FLAG_NONE,
    make_new, mongoose_mqttclient_make_new,
    locals_dict, &mongoose_mqttclient_locals_dict
    );



STATIC const mp_rom_map_elem_t mongoose_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_mongoose) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mongoose_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_poll), MP_ROM_PTR(&mongoose_poll_obj) },
    { MP_ROM_QSTR(MP_QSTR_MQTTclient), MP_ROM_PTR(&mongoose_type_mqttclient) },
};
STATIC MP_DEFINE_CONST_DICT(mongoose_module_globals, mongoose_module_globals_table);

const mp_obj_module_t mongoose_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mongoose_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_mongoose, mongoose_module);
