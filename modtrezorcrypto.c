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

#if MICROPY_PY_TREZORCRYPTO

#include "modtrezorcrypto-aes.h"
#include "modtrezorcrypto-crc.h"
#include "modtrezorcrypto-bip32.h"
#include "modtrezorcrypto-bip39.h"
#include "modtrezorcrypto-blake256.h"
#include "modtrezorcrypto-blake2b.h"
#include "modtrezorcrypto-blake2s.h"
#include "modtrezorcrypto-chacha20poly1305.h"
#include "modtrezorcrypto-curve25519.h"
#include "modtrezorcrypto-ed25519.h"
#include "modtrezorcrypto-nist256p1.h"
#include "modtrezorcrypto-pbkdf2.h"
#include "modtrezorcrypto-random.h"
#include "modtrezorcrypto-rfc6979.h"
#include "modtrezorcrypto-ripemd160.h"
#include "modtrezorcrypto-secp256k1.h"
#include "modtrezorcrypto-sha1.h"
#include "modtrezorcrypto-sha256.h"
#include "modtrezorcrypto-sha512.h"
#if USE_KECCAK
#include "modtrezorcrypto-sha3-256.h"
#include "modtrezorcrypto-sha3-512.h"
#endif

STATIC const mp_rom_map_elem_t mp_module_trezorcrypto_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_trezorcrypto) },
    { MP_ROM_QSTR(MP_QSTR_AES), MP_ROM_PTR(&mod_trezorcrypto_AES_type) },
    { MP_ROM_QSTR(MP_QSTR_crc), MP_ROM_PTR(&mod_trezorcrypto_crc_module) },
    { MP_ROM_QSTR(MP_QSTR_bip32), MP_ROM_PTR(&mod_trezorcrypto_bip32_module) },
    { MP_ROM_QSTR(MP_QSTR_bip39), MP_ROM_PTR(&mod_trezorcrypto_bip39_module) },
#if 1
    { MP_ROM_QSTR(MP_QSTR_blake256), MP_ROM_PTR(&mod_trezorcrypto_Blake256_type) },
    { MP_ROM_QSTR(MP_QSTR_blake2b), MP_ROM_PTR(&mod_trezorcrypto_Blake2b_type) },
    { MP_ROM_QSTR(MP_QSTR_blake2s), MP_ROM_PTR(&mod_trezorcrypto_Blake2s_type) },
    { MP_ROM_QSTR(MP_QSTR_chacha20poly1305), MP_ROM_PTR(&mod_trezorcrypto_ChaCha20Poly1305_type) },
    { MP_ROM_QSTR(MP_QSTR_curve25519), MP_ROM_PTR(&mod_trezorcrypto_curve25519_module) },
    { MP_ROM_QSTR(MP_QSTR_ed25519), MP_ROM_PTR(&mod_trezorcrypto_ed25519_module) },
    { MP_ROM_QSTR(MP_QSTR_nist256p1), MP_ROM_PTR(&mod_trezorcrypto_nist256p1_module) },
#endif
    { MP_ROM_QSTR(MP_QSTR_pbkdf2), MP_ROM_PTR(&mod_trezorcrypto_Pbkdf2_type) },
    { MP_ROM_QSTR(MP_QSTR_random), MP_ROM_PTR(&mod_trezorcrypto_random_module) },
    { MP_ROM_QSTR(MP_QSTR_rfc6979), MP_ROM_PTR(&mod_trezorcrypto_Rfc6979_type) },
    { MP_ROM_QSTR(MP_QSTR_ripemd160), MP_ROM_PTR(&mod_trezorcrypto_Ripemd160_type) },
    { MP_ROM_QSTR(MP_QSTR_secp256k1), MP_ROM_PTR(&mod_trezorcrypto_secp256k1_module) },
    { MP_ROM_QSTR(MP_QSTR_sha1), MP_ROM_PTR(&mod_trezorcrypto_Sha1_type) },
    { MP_ROM_QSTR(MP_QSTR_sha256), MP_ROM_PTR(&mod_trezorcrypto_Sha256_type) },
    { MP_ROM_QSTR(MP_QSTR_sha512), MP_ROM_PTR(&mod_trezorcrypto_Sha512_type) },
#if USE_KECCAK
    { MP_ROM_QSTR(MP_QSTR_sha3_256), MP_ROM_PTR(&mod_trezorcrypto_Sha3_256_type) },
    { MP_ROM_QSTR(MP_QSTR_sha3_512), MP_ROM_PTR(&mod_trezorcrypto_Sha3_512_type) },
#endif
};
STATIC MP_DEFINE_CONST_DICT(mp_module_trezorcrypto_globals, mp_module_trezorcrypto_globals_table);

const mp_obj_module_t mp_module_trezorcrypto = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_trezorcrypto_globals,
};

#endif // MICROPY_PY_TREZORCRYPTO
