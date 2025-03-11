#include "py/obj.h"
#include "py/runtime.h"
#include "msx_hw.h"

static mp_obj_t msx_vpoke(mp_obj_t addr_in, mp_obj_t val_in) {
    // Validar que los parámetros sean enteros
    if (!mp_obj_is_int(addr_in) || !mp_obj_is_int(val_in)) {
        mp_raise_TypeError(MP_ERROR_TEXT("vpoke requiere enteros"));
    }

    // Convertir los parámetros a valores numéricos
    uint16_t addr = mp_obj_get_int(addr_in);  // 16 bits
    uint8_t val = mp_obj_get_int(val_in);     // 8 bits

    vpoke(addr, val);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(msx_vpoke_obj, msx_vpoke);

static mp_obj_t msx_outp(mp_obj_t addr_in, mp_obj_t val_in) {
    // Validar que los parámetros sean enteros
    if (!mp_obj_is_int(addr_in) || !mp_obj_is_int(val_in)) {
        mp_raise_TypeError(MP_ERROR_TEXT("outp requiere enteros"));
    }

    // Convertir los parámetros a valores numéricos
    uint16_t addr = mp_obj_get_int(addr_in);  // 16 bits
    uint8_t val = mp_obj_get_int(val_in);     // 8 bits

    z80_out(addr, val);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(msx_outp_obj, msx_outp);

static mp_obj_t msx_vpeek(mp_obj_t addr_in) {
    // Validar que el parámetro sea un entero
    if (!mp_obj_is_int(addr_in)) {
        mp_raise_TypeError(MP_ERROR_TEXT("vpeek requiere un entero"));
    }

    // Convertir el parámetro a uint16_t
    uint16_t addr = mp_obj_get_int(addr_in);

    // Leer el valor de VRAM
    uint8_t val = vpeek(addr);

    // Retornar el valor como objeto de MicroPython
    return mp_obj_new_int(val);
}
static MP_DEFINE_CONST_FUN_OBJ_1(msx_vpeek_obj, msx_vpeek);

static mp_obj_t msx_inp(mp_obj_t addr_in) {
    // Validar que el parámetro sea un entero
    if (!mp_obj_is_int(addr_in)) {
        mp_raise_TypeError(MP_ERROR_TEXT("inp requiere un entero"));
    }

    // Convertir el parámetro a uint16_t
    uint16_t addr = mp_obj_get_int(addr_in);

    // Leer el valor de VRAM
    uint8_t val = z80_in(addr);

    // Retornar el valor como objeto de MicroPython
    return mp_obj_new_int(val);
}
static MP_DEFINE_CONST_FUN_OBJ_1(msx_inp_obj, msx_inp);

static const mp_rom_map_elem_t msx_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_vpoke), MP_ROM_PTR(&msx_vpoke_obj) },
    { MP_ROM_QSTR(MP_QSTR_outp), MP_ROM_PTR(&msx_outp_obj) },
    { MP_ROM_QSTR(MP_QSTR_vpeek), MP_ROM_PTR(&msx_vpeek_obj) },
    { MP_ROM_QSTR(MP_QSTR_inp), MP_ROM_PTR(&msx_inp_obj) },
};

static MP_DEFINE_CONST_DICT(msx_module_globals, msx_module_globals_table);

const mp_obj_module_t msx_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&msx_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_msx, msx_module);
