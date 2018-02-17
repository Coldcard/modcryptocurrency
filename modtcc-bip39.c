/*
 * Copyright (c) Pavol Rusnak, SatoshiLabs
 *
 * Licensed under TREZOR License
 * see LICENSE file for details
 */

#include "py/objstr.h"

#include "bip39.h"

/// def lookup(idx: int) -> str:
///     '''
///     Return the n-th word from the wordlist.
///     '''
STATIC mp_obj_t mod_tcc_bip39_lookup(mp_obj_t idx_obj)
{
    int idx = mp_obj_get_int(idx_obj);

    // we've all read the BIP, so obviously 12 bits only here.
    if(idx < 0 || idx > 0x7ff) {
        mp_raise_ValueError("wordlist range");
    }

    const char * const *wordlist = mnemonic_wordlist();
    const char *w = wordlist[idx];

    return mp_obj_new_str(w, strlen(w));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_tcc_bip39_lookup_obj, mod_tcc_bip39_lookup);

/// def find_word(prefix: str) -> Optional[str]:
///     '''
///     Return the first word from the wordlist starting with prefix.
///     '''
STATIC mp_obj_t mod_trezorcrypto_bip39_find_word(mp_obj_t prefix)
{
    mp_buffer_info_t pfx;
    mp_get_buffer_raise(prefix, &pfx, MP_BUFFER_READ);
    if (pfx.len == 0) {
        mp_raise_ValueError("Invalid word prefix");
    }
    for (const char * const *w = mnemonic_wordlist(); *w != 0; w++) {
        if (strncmp(*w, pfx.buf, pfx.len) == 0) {
            return mp_obj_new_str(*w, strlen(*w));
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_bip39_find_word_obj, mod_trezorcrypto_bip39_find_word);

/// def complete_word(prefix: str) -> int:
///     '''
///     Return possible 1-letter suffixes for given word prefix.
///     Result is a bitmask, with 'a' on the lowest bit, 'b' on the second lowest, etc.
///     '''
STATIC mp_obj_t mod_trezorcrypto_bip39_complete_word(mp_obj_t prefix)
{
    mp_buffer_info_t pfx;
    mp_get_buffer_raise(prefix, &pfx, MP_BUFFER_READ);
    if (pfx.len == 0) {
        mp_raise_ValueError("Invalid word prefix");
    }
    uint32_t res = 0;
    uint8_t bit;
    const char *word;
    const char *const *wlist;
    for (wlist = mnemonic_wordlist(); *wlist != 0; wlist++) {
        word = *wlist;
        if (strncmp(word, pfx.buf, pfx.len) == 0 && strlen(word) > pfx.len) {
            bit = word[pfx.len] - 'a';
            res |= 1 << bit;
        }
    }
    return mp_obj_new_int(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_bip39_complete_word_obj, mod_trezorcrypto_bip39_complete_word);

/// def generate(strength: int) -> str:
///     '''
///     Generate a mnemonic of given strength (128, 160, 192, 224 and 256 bits).
///     '''
STATIC mp_obj_t mod_trezorcrypto_bip39_generate(mp_obj_t strength) {
    int bits = mp_obj_get_int(strength);
    if (bits % 32 || bits < 128 || bits > 256) {
        mp_raise_ValueError("Invalid bit strength (only 128, 160, 192, 224 and 256 values are allowed)");
    }
    const char *mnemo = mnemonic_generate(bits);
    return mp_obj_new_str(mnemo, strlen(mnemo));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_bip39_generate_obj, mod_trezorcrypto_bip39_generate);

/// def from_data(data: bytes) -> str:
///     '''
///     Generate a mnemonic from given data (of 16, 20, 24, 28 and 32 bytes).
///     '''
STATIC mp_obj_t mod_trezorcrypto_bip39_from_data(mp_obj_t data) {
    mp_buffer_info_t bin;
    mp_get_buffer_raise(data, &bin, MP_BUFFER_READ);
    if (bin.len % 4 || bin.len < 16 || bin.len > 32) {
        mp_raise_ValueError("Invalid data length (only 16, 20, 24, 28 and 32 bytes are allowed)");
    }
    const char *mnemo = mnemonic_from_data(bin.buf, bin.len);
    return mp_obj_new_str(mnemo, strlen(mnemo));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_bip39_from_data_obj, mod_trezorcrypto_bip39_from_data);

/// def check(mnemonic: str) -> bool:
///     '''
///     Check whether given mnemonic is valid.
///     '''
STATIC mp_obj_t mod_trezorcrypto_bip39_check(mp_obj_t mnemonic) {
    mp_buffer_info_t text;
    mp_get_buffer_raise(mnemonic, &text, MP_BUFFER_READ);
    return (text.len > 0 && mnemonic_check(text.buf)) ? mp_const_true : mp_const_false;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_bip39_check_obj, mod_trezorcrypto_bip39_check);

/// def seed(mnemonic: str, passphrase: str) -> bytes:
///     '''
///     Generate seed from mnemonic and passphrase.
///     '''
STATIC mp_obj_t mod_trezorcrypto_bip39_seed(mp_obj_t mnemonic, mp_obj_t passphrase) {
    mp_buffer_info_t mnemo;
    mp_buffer_info_t phrase;
    mp_get_buffer_raise(mnemonic, &mnemo, MP_BUFFER_READ);
    mp_get_buffer_raise(passphrase, &phrase, MP_BUFFER_READ);
    uint8_t seed[64];
    const char *pmnemonic = mnemo.len > 0 ? mnemo.buf : "";
    const char *ppassphrase = phrase.len > 0 ? phrase.buf : "";
    mnemonic_to_seed(pmnemonic, ppassphrase, seed, NULL); // no callback for now
    return mp_obj_new_bytes(seed, sizeof(seed));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_trezorcrypto_bip39_seed_obj, mod_trezorcrypto_bip39_seed);

STATIC const mp_rom_map_elem_t mod_trezorcrypto_bip39_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_bip39) },
    { MP_ROM_QSTR(MP_QSTR_find_word), MP_ROM_PTR(&mod_trezorcrypto_bip39_find_word_obj) },
    { MP_ROM_QSTR(MP_QSTR_complete_word), MP_ROM_PTR(&mod_trezorcrypto_bip39_complete_word_obj) },
    { MP_ROM_QSTR(MP_QSTR_generate), MP_ROM_PTR(&mod_trezorcrypto_bip39_generate_obj) },
    { MP_ROM_QSTR(MP_QSTR_from_data), MP_ROM_PTR(&mod_trezorcrypto_bip39_from_data_obj) },
    { MP_ROM_QSTR(MP_QSTR_check), MP_ROM_PTR(&mod_trezorcrypto_bip39_check_obj) },
    { MP_ROM_QSTR(MP_QSTR_seed), MP_ROM_PTR(&mod_trezorcrypto_bip39_seed_obj) },
    { MP_ROM_QSTR(MP_QSTR_lookup), MP_ROM_PTR(&mod_tcc_bip39_lookup_obj) },
};
STATIC MP_DEFINE_CONST_DICT(mod_trezorcrypto_bip39_globals, mod_trezorcrypto_bip39_globals_table);

STATIC const mp_obj_module_t mod_trezorcrypto_bip39_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mod_trezorcrypto_bip39_globals,
};
