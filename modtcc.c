/*
 * Copyright (c) Pavol Rusnak, SatoshiLabs
 *
 * Licensed under TREZOR License
 * see LICENSE file for details
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "py/runtime.h"

// NOTE: These are not header files, but C files.

#include "modtcc-aes.c"
#include "modtcc-crc.c"
#include "modtcc-bip32.c"
#include "modtcc-bip39.c"
#include "modtcc-nist256p1.c"
#include "modtcc-pbkdf2.c"
#include "modtcc-random.c"
#include "modtcc-rfc6979.c"
#include "modtcc-ripemd160.c"
#include "modtcc-secp256k1.c"
#include "modtcc-sha1.c"
#include "modtcc-sha256.c"
#include "modtcc-sha512.c"
#include "modtcc-codecs.c"

#if 1
#include "modtcc-blake256.c"
#include "modtcc-blake2b.c"
#include "modtcc-blake2s.c"
#include "modtcc-chacha20poly1305.c"
#include "modtcc-curve25519.c"
#include "modtcc-ed25519.c"
#endif
#if USE_KECCAK
#include "modtcc-sha3-256.c"
#include "modtcc-sha3-512.c"
#endif

STATIC const mp_rom_map_elem_t mp_module_tcc_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_trezorcrypto) },
    { MP_ROM_QSTR(MP_QSTR_AES), MP_ROM_PTR(&mod_trezorcrypto_AES_type) },
    { MP_ROM_QSTR(MP_QSTR_crc), MP_ROM_PTR(&mod_trezorcrypto_crc_module) },
    { MP_ROM_QSTR(MP_QSTR_bip32), MP_ROM_PTR(&mod_trezorcrypto_bip32_module) },
    { MP_ROM_QSTR(MP_QSTR_bip39), MP_ROM_PTR(&mod_trezorcrypto_bip39_module) },
    { MP_ROM_QSTR(MP_QSTR_nist256p1), MP_ROM_PTR(&mod_trezorcrypto_nist256p1_module) },
    { MP_ROM_QSTR(MP_QSTR_pbkdf2), MP_ROM_PTR(&mod_trezorcrypto_Pbkdf2_type) },
    { MP_ROM_QSTR(MP_QSTR_random), MP_ROM_PTR(&mod_trezorcrypto_random_module) },
    { MP_ROM_QSTR(MP_QSTR_rfc6979), MP_ROM_PTR(&mod_trezorcrypto_Rfc6979_type) },
    { MP_ROM_QSTR(MP_QSTR_ripemd160), MP_ROM_PTR(&mod_trezorcrypto_Ripemd160_type) },
    { MP_ROM_QSTR(MP_QSTR_secp256k1), MP_ROM_PTR(&mod_trezorcrypto_secp256k1_module) },
    { MP_ROM_QSTR(MP_QSTR_sha1), MP_ROM_PTR(&mod_trezorcrypto_Sha1_type) },
    { MP_ROM_QSTR(MP_QSTR_sha256), MP_ROM_PTR(&mod_trezorcrypto_Sha256_type) },
    { MP_ROM_QSTR(MP_QSTR_sha512), MP_ROM_PTR(&mod_trezorcrypto_Sha512_type) },
    { MP_ROM_QSTR(MP_QSTR_codecs), MP_ROM_PTR(&modtcc_codecs_module) },
#if 1
    { MP_ROM_QSTR(MP_QSTR_blake256), MP_ROM_PTR(&mod_trezorcrypto_Blake256_type) },
    { MP_ROM_QSTR(MP_QSTR_blake2b), MP_ROM_PTR(&mod_trezorcrypto_Blake2b_type) },
    { MP_ROM_QSTR(MP_QSTR_blake2s), MP_ROM_PTR(&mod_trezorcrypto_Blake2s_type) },
    { MP_ROM_QSTR(MP_QSTR_chacha20poly1305), MP_ROM_PTR(&mod_trezorcrypto_ChaCha20Poly1305_type) },
    { MP_ROM_QSTR(MP_QSTR_curve25519), MP_ROM_PTR(&mod_trezorcrypto_curve25519_module) },
    { MP_ROM_QSTR(MP_QSTR_ed25519), MP_ROM_PTR(&mod_trezorcrypto_ed25519_module) },
#endif
#if USE_KECCAK
    { MP_ROM_QSTR(MP_QSTR_sha3_256), MP_ROM_PTR(&mod_trezorcrypto_Sha3_256_type) },
    { MP_ROM_QSTR(MP_QSTR_sha3_512), MP_ROM_PTR(&mod_trezorcrypto_Sha3_512_type) },
#endif
};
STATIC MP_DEFINE_CONST_DICT(mp_module_tcc_globals, mp_module_tcc_globals_table);

const mp_obj_module_t mp_module_tcc = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_tcc_globals,
};

