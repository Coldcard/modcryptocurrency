/*
 * Copyright (c) 2018 Coinkite Inc.
 *
 * Licensed under GNU License
 * see LICENSE file for details
 * 
 * 
 * Various encodes/decoders/serializers: base58, base32, bech32, etc.
 *
 */

#include "py/objstr.h"

#include "base58.h"
#include "base32.h"
#include "hasher.h"

/*
int base58_encode_check(const uint8_t *data, int len, HasherType hasher_type, char *str, int strsize);
int base58_decode_check(const char *str, HasherType hasher_type, uint8_t *data, int datalen);

char *base32_encode(const uint8_t *in, size_t inlen, char *out, size_t outlen, const char *alphabet);
uint8_t *base32_decode(const char *in, size_t inlen, uint8_t *out, size_t outlen, const char *alphabet);
*/

//
// Base 58
//

STATIC mp_obj_t modtcc_b58_encode(mp_obj_t data)
{
    mp_buffer_info_t buf;
    mp_get_buffer_raise(data, &buf, MP_BUFFER_READ);
    if (buf.len == 0) {
        // there is an encoding for empty string (4 bytes of fixed checksum) but not useful
        mp_raise_ValueError(NULL);
    }

    vstr_t vstr;
    vstr_init_len(&vstr, (buf.len*2)+10);

    int rl = base58_encode_check(buf.buf, buf.len, HASHER_SHA2, vstr.buf, vstr.len);

    if(rl < 1) {
        // unlikely
        mp_raise_ValueError(NULL);
    }

    vstr.len = rl-1;        // strip NUL
    
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(modtcc_b58_encode_obj, modtcc_b58_encode);

STATIC mp_obj_t modtcc_b58_decode(mp_obj_t enc)
{
    const char *s = mp_obj_str_get_str(enc);

    uint8_t tmp[128];

    int rl = base58_decode_check(s, HASHER_SHA2, tmp, sizeof(tmp));

    if(rl <= 0) {
        // transcription error from user is very likely
        mp_raise_ValueError("corrupt base58");
    }
    
    return mp_obj_new_bytes(tmp, rl);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(modtcc_b58_decode_obj, modtcc_b58_decode);

//
// Base 32
//

STATIC mp_obj_t modtcc_b32_encode(mp_obj_t data)
{
    mp_buffer_info_t buf;
    mp_get_buffer_raise(data, &buf, MP_BUFFER_READ);
    if (buf.len == 0) {
        return mp_const_empty_bytes;
    }

    vstr_t vstr;
    vstr_init_len(&vstr, (buf.len*2)+10);

    char *last = base32_encode(buf.buf, buf.len, vstr.buf, vstr.len, BASE32_ALPHABET_RFC4648);

    if(!last) {
        // unlikely
        mp_raise_ValueError(NULL);
    }

    vstr.len = last - vstr.buf;        // strips NUL
    
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(modtcc_b32_encode_obj, modtcc_b32_encode);

STATIC mp_obj_t modtcc_b32_decode(mp_obj_t enc)
{
    const char *s = mp_obj_str_get_str(enc);

    uint8_t tmp[256];

    uint8_t *last = base32_decode(s, strlen(s), tmp, sizeof(tmp), BASE32_ALPHABET_RFC4648);

    if(!last) {
        // transcription error from user is very likely
        mp_raise_ValueError("corrupt base32");
    }
    
    return mp_obj_new_bytes(tmp, last-tmp);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(modtcc_b32_decode_obj, modtcc_b32_decode);


STATIC const mp_rom_map_elem_t modtcc_codecs_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_codecs) },
    { MP_ROM_QSTR(MP_QSTR_b58_encode), MP_ROM_PTR(&modtcc_b58_encode_obj) },
    { MP_ROM_QSTR(MP_QSTR_b58_decode), MP_ROM_PTR(&modtcc_b58_decode_obj) },
    { MP_ROM_QSTR(MP_QSTR_b32_encode), MP_ROM_PTR(&modtcc_b32_encode_obj) },
    { MP_ROM_QSTR(MP_QSTR_b32_decode), MP_ROM_PTR(&modtcc_b32_decode_obj) },
};
STATIC MP_DEFINE_CONST_DICT(modtcc_codecs_globals, modtcc_codecs_globals_table);

STATIC const mp_obj_module_t modtcc_codecs_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&modtcc_codecs_globals,
};

// EOF
