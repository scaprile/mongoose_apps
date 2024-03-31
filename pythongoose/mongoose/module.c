#include "py/runtime.h"

extern void mgmp_init(void);
extern void mgmp_poll(int);

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

STATIC const mp_rom_map_elem_t mongoose_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_mongoose) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mongoose_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_poll), MP_ROM_PTR(&mongoose_poll_obj) },
};
STATIC MP_DEFINE_CONST_DICT(mongoose_module_globals, mongoose_module_globals_table);

const mp_obj_module_t mongoose_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mongoose_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_mongoose, mongoose_module);
