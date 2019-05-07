/*
 * Copyright (c) Jan Pochyla, SatoshiLabs
 *
 * Licensed under TREZOR License
 * see LICENSE file for details
 */

#include "py/objstr.h"

#include "bip32.h"
#include "curves.h"

/// class HDNode:
///     '''
///     BIP0032 HD node structure.
///     '''
typedef struct _mp_obj_HDNode_t {
    mp_obj_base_t base;
    uint32_t fingerprint;
    bool have_private;
    HDNode hdnode;
} mp_obj_HDNode_t;

STATIC const mp_obj_type_t mod_trezorcrypto_HDNode_type;

#define XPUB_MAXLEN 128
#define ADDRESS_MAXLEN 36

/// def __init__(self,
///              depth: int,
///              fingerprint: int,
///              child_num: int,
///              chain_code: bytes,
///              private_key: bytes = None,
///              public_key: bytes = None,
///              curve_name: str = None) -> None:
///     '''
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {

    // NOTE: mp_const_empty_bytes does not have NULL buf value! Look at the .len

    STATIC const mp_arg_t allowed_args[] = {
        { MP_QSTR_depth,        MP_ARG_REQUIRED | MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_fingerprint,   MP_ARG_REQUIRED | MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_child_num,    MP_ARG_REQUIRED | MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_chain_code,   MP_ARG_REQUIRED | MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_empty_bytes} },
        { MP_QSTR_private_key,                    MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_empty_bytes} },
        { MP_QSTR_public_key,                     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_empty_bytes} },
        { MP_QSTR_curve_name,                     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_empty_bytes} },
    };
    mp_arg_val_t vals[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, vals);

    mp_buffer_info_t chain_code;
    mp_buffer_info_t private_key;
    mp_buffer_info_t public_key;
    mp_buffer_info_t curve_name;
    const mp_int_t depth      = vals[0].u_int;
    const mp_int_t fingerprint = vals[1].u_int;
    const mp_int_t child_num  = vals[2].u_int;
    mp_get_buffer_raise(vals[3].u_obj, &chain_code, MP_BUFFER_READ);
    mp_get_buffer_raise(vals[4].u_obj, &private_key, MP_BUFFER_READ);
    mp_get_buffer_raise(vals[5].u_obj, &public_key, MP_BUFFER_READ);
    mp_get_buffer_raise(vals[6].u_obj, &curve_name, MP_BUFFER_READ);

    if (chain_code.len && 32 != chain_code.len) {
        mp_raise_ValueError("chain_code is invalid");
    }
    if (!public_key.len && !private_key.len) {
        mp_raise_ValueError("either public_key or private_key is required");
    }
    if (private_key.len && private_key.len != 32) {
        mp_raise_ValueError("private_key is invalid");
    }
    if (public_key.len && public_key.len != 33) {
        mp_raise_ValueError("public_key is invalid");
    }

    const curve_info *curve = NULL;
    if (!curve_name.len) {
        curve = get_curve_by_name(SECP256K1_NAME);
    } else {
        curve = get_curve_by_name(curve_name.buf);
    }
    if (NULL == curve) {
        mp_raise_ValueError("curve_name is invalid");
    }

    mp_obj_HDNode_t *o = m_new_obj(mp_obj_HDNode_t);
    o->base.type = type;

    o->fingerprint = (uint32_t)fingerprint;
    o->hdnode.depth = (uint32_t)depth;
    o->hdnode.child_num = (uint32_t)child_num;
    if (chain_code.len) {
        memcpy(o->hdnode.chain_code, chain_code.buf, 32);
    } else {
        memset(o->hdnode.chain_code, 0, 32);
    }
    if (private_key.len) {
        memcpy(o->hdnode.private_key, private_key.buf, 32);
        o->have_private = true;
    } else {
        memset(o->hdnode.private_key, 0, 32);
        o->have_private = false;
    }
    if (public_key.len) {
        memcpy(o->hdnode.public_key, public_key.buf, 33);
    } else {
        memset(o->hdnode.public_key, 0, 33);
    }
    o->hdnode.curve = curve;

    return MP_OBJ_FROM_PTR(o);
}

/// def derive(self, index: int) -> None:
///     '''
///     Derive a BIP0032 child node in place.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_derive(mp_obj_t self, mp_obj_t index) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);
    uint32_t i = mp_obj_get_int_truncated(index);
    uint32_t fp = hdnode_fingerprint(&o->hdnode);
    int res;

    if(!o->have_private) {
        // must do a public derivation when we have no private key
        res = hdnode_public_ckd(&o->hdnode, i);
    } else {
        res = hdnode_private_ckd(&o->hdnode, i);
    }

    if (!res) {
        memset(&o->hdnode, 0, sizeof(o->hdnode));
        mp_raise_ValueError("Failed to derive");
    }

    o->fingerprint = fp;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_trezorcrypto_HDNode_derive_obj, mod_trezorcrypto_HDNode_derive);

/// def derive_path(self, path: List[int]) -> None:
///     '''
///     Go through a list of indexes and iteratively derive a child node in place.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_derive_path(mp_obj_t self, mp_obj_t path) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);

    // get path objects and length
    size_t plen;
    mp_obj_t *pitems;
    mp_obj_get_array(path, &plen, &pitems);
    if (plen > 32) {
        mp_raise_ValueError("Path cannot be longer than 32 indexes");
    }

    // convert path to int array
    uint32_t pi;
    uint32_t pints[plen];
    for (pi = 0; pi < plen; pi++) {
        if (!MP_OBJ_IS_INT(pitems[pi])) {
            mp_raise_TypeError("Index has to be int");
        }
        pints[pi] = mp_obj_get_int_truncated(pitems[pi]);
    }

    if (!hdnode_private_ckd_cached(&o->hdnode, pints, plen, &o->fingerprint)) {
        // derivation failed, reset the state and raise
        o->fingerprint = 0;
        memset(&o->hdnode, 0, sizeof(o->hdnode));
        mp_raise_ValueError("Failed to derive path");
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_trezorcrypto_HDNode_derive_path_obj, mod_trezorcrypto_HDNode_derive_path);

STATIC mp_obj_t serialize_public_private(mp_obj_t self, bool use_public, uint32_t version) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);
    char xpub[XPUB_MAXLEN];
    int written;
    if (use_public) {
        hdnode_fill_public_key(&o->hdnode);
        written = hdnode_serialize_public(&o->hdnode, o->fingerprint, version, xpub, XPUB_MAXLEN);
    } else {
        written = hdnode_serialize_private(&o->hdnode, o->fingerprint, version, xpub, XPUB_MAXLEN);
    }
    if (written <= 0) {
        mp_raise_ValueError("Failed to serialize");
    }
    return mp_obj_new_str(xpub, written - 1);  // written includes 0 at the end
}

/// def serialize_public(self, version: int) -> str:
///     '''
///     Serialize the public info from HD node to base58 string.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_serialize_public(mp_obj_t self, mp_obj_t version) {
    uint32_t ver = mp_obj_get_int_truncated(version);
    return serialize_public_private(self, true, ver);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_trezorcrypto_HDNode_serialize_public_obj, mod_trezorcrypto_HDNode_serialize_public);

/// def serialize_private(self, version: int) -> str:
///     '''
///     Serialize the private info HD node to base58 string.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_serialize_private(mp_obj_t self, mp_obj_t version) {
    uint32_t ver = mp_obj_get_int_truncated(version);
    return serialize_public_private(self, false, ver);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_trezorcrypto_HDNode_serialize_private_obj, mod_trezorcrypto_HDNode_serialize_private);

/// def clone(self) -> HDNode:
///     '''
///     Returns a copy of the HD node.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_clone(mp_obj_t self) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);
    mp_obj_HDNode_t *copy = m_new_obj(mp_obj_HDNode_t);
    copy->base.type = &mod_trezorcrypto_HDNode_type;
    copy->hdnode = o->hdnode;
    copy->have_private = o->have_private;
    copy->fingerprint = o->fingerprint;
    return MP_OBJ_FROM_PTR(copy);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_HDNode_clone_obj, mod_trezorcrypto_HDNode_clone);

/// def blank(self) -> HDNode:
///     '''
///     Blank out sensitive values in self. Object useless afterwards.
///     '''
STATIC mp_obj_t modtcc_HDNode_blank(mp_obj_t self) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);

    memset(&o->hdnode, 0, sizeof(HDNode));
    o->fingerprint = 0;
    o->have_private = false;

    return mp_const_none;

}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(modtcc_HDNode_blank_obj, modtcc_HDNode_blank);

/// def depth(self) -> int:
///     '''
///     Returns a depth of the HD node.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_depth(mp_obj_t self) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);
    return mp_obj_new_int_from_uint(o->hdnode.depth);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_HDNode_depth_obj, mod_trezorcrypto_HDNode_depth);

/// def fingerprint(self) -> int:
///     '''
///     Returns a fingerprint of the HD node (hash of the parent public key).
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_fingerprint(mp_obj_t self) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);
    return mp_obj_new_int_from_uint(o->fingerprint);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_HDNode_fingerprint_obj, mod_trezorcrypto_HDNode_fingerprint);

/// def my_fingerprint(self) -> int:
///     '''
///     Returns a fingerprint of this HD node (hash of its public key)... not the parent.
///     NOTE: this is a little-endian 32-bit number, not BE32 used elsewhere in this library.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_my_fingerprint(mp_obj_t self) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);
    uint32_t fp = hdnode_fingerprint(&o->hdnode);
    return mp_obj_new_int_from_uint(__builtin_bswap32(fp));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_HDNode_my_fingerprint_obj, mod_trezorcrypto_HDNode_my_fingerprint);

/// def child_num(self) -> int:
///     '''
///     Returns a child index of the HD node.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_child_num(mp_obj_t self) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);
    return mp_obj_new_int_from_uint(o->hdnode.child_num);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_HDNode_child_num_obj, mod_trezorcrypto_HDNode_child_num);

/// def chain_code(self) -> bytes:
///     '''
///     Returns a chain code of the HD node.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_chain_code(mp_obj_t self) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);
    return mp_obj_new_bytes(o->hdnode.chain_code, sizeof(o->hdnode.chain_code));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_HDNode_chain_code_obj, mod_trezorcrypto_HDNode_chain_code);

/// def private_key(self) -> bytes:
///     '''
///     Returns a private key of the HD node.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_private_key(mp_obj_t self) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);

    if(!o->have_private) {
        return mp_const_none;
    } else {
        return mp_obj_new_bytes(o->hdnode.private_key, sizeof(o->hdnode.private_key));
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_HDNode_private_key_obj, mod_trezorcrypto_HDNode_private_key);

/// def public_key(self) -> bytes:
///     '''
///     Returns a public key of the HD node.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_public_key(mp_obj_t self) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);
    hdnode_fill_public_key(&o->hdnode);
    return mp_obj_new_bytes(o->hdnode.public_key, sizeof(o->hdnode.public_key));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_HDNode_public_key_obj, mod_trezorcrypto_HDNode_public_key);

/// def address(self, version: int) -> str:
///     '''
///     Compute a base58-encoded address string from the HD node.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_address(mp_obj_t self, mp_obj_t version) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);

    uint32_t v = mp_obj_get_int_truncated(version);
    char address[ADDRESS_MAXLEN];
    hdnode_get_address(&o->hdnode, v, address, ADDRESS_MAXLEN);
    return mp_obj_new_str(address, strlen(address));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_trezorcrypto_HDNode_address_obj, mod_trezorcrypto_HDNode_address);

/// def address_raw(self) -> bytes[20]:
///     '''
///     Compute a ripemd160-hash of hash(pubkey). Always 20 bytes of binary.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_address_raw(mp_obj_t self) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);
    // API requires a version, but we'll use zero and remove it.
    uint8_t raw[21];
    hdnode_get_address_raw(&o->hdnode, 0x0, raw);
    return mp_obj_new_bytes(raw+1, 20);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_HDNode_address_raw_obj, mod_trezorcrypto_HDNode_address_raw);

#if USE_ETHEREUM
/// def ethereum_pubkeyhash(self) -> bytes:
///     '''
///     Compute an Ethereum pubkeyhash (aka address) from the HD node.
///     '''
STATIC mp_obj_t mod_trezorcrypto_HDNode_ethereum_pubkeyhash(mp_obj_t self) {
    mp_obj_HDNode_t *o = MP_OBJ_TO_PTR(self);

    uint8_t pkh[20];
    hdnode_get_ethereum_pubkeyhash(&o->hdnode, pkh);
    return mp_obj_new_bytes(pkh, sizeof(pkh));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_trezorcrypto_HDNode_ethereum_pubkeyhash_obj, mod_trezorcrypto_HDNode_ethereum_pubkeyhash);
#endif

STATIC const mp_rom_map_elem_t mod_trezorcrypto_HDNode_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_derive), MP_ROM_PTR(&mod_trezorcrypto_HDNode_derive_obj) },
    { MP_ROM_QSTR(MP_QSTR_derive_path), MP_ROM_PTR(&mod_trezorcrypto_HDNode_derive_path_obj) },
    { MP_ROM_QSTR(MP_QSTR_serialize_private), MP_ROM_PTR(&mod_trezorcrypto_HDNode_serialize_private_obj) },
    { MP_ROM_QSTR(MP_QSTR_serialize_public), MP_ROM_PTR(&mod_trezorcrypto_HDNode_serialize_public_obj) },

    { MP_ROM_QSTR(MP_QSTR_clone), MP_ROM_PTR(&mod_trezorcrypto_HDNode_clone_obj) },
    { MP_ROM_QSTR(MP_QSTR_depth), MP_ROM_PTR(&mod_trezorcrypto_HDNode_depth_obj) },
    { MP_ROM_QSTR(MP_QSTR_fingerprint), MP_ROM_PTR(&mod_trezorcrypto_HDNode_fingerprint_obj) },
    { MP_ROM_QSTR(MP_QSTR_my_fingerprint), MP_ROM_PTR(&mod_trezorcrypto_HDNode_my_fingerprint_obj) },
    { MP_ROM_QSTR(MP_QSTR_child_num), MP_ROM_PTR(&mod_trezorcrypto_HDNode_child_num_obj) },
    { MP_ROM_QSTR(MP_QSTR_chain_code), MP_ROM_PTR(&mod_trezorcrypto_HDNode_chain_code_obj) },
    { MP_ROM_QSTR(MP_QSTR_private_key), MP_ROM_PTR(&mod_trezorcrypto_HDNode_private_key_obj) },
    { MP_ROM_QSTR(MP_QSTR_public_key), MP_ROM_PTR(&mod_trezorcrypto_HDNode_public_key_obj) },
    { MP_ROM_QSTR(MP_QSTR_address), MP_ROM_PTR(&mod_trezorcrypto_HDNode_address_obj) },
    { MP_ROM_QSTR(MP_QSTR_address_raw), MP_ROM_PTR(&mod_trezorcrypto_HDNode_address_raw_obj) },
    { MP_ROM_QSTR(MP_QSTR_blank), MP_ROM_PTR(&modtcc_HDNode_blank_obj) },
#if USE_ETHEREUM
    { MP_ROM_QSTR(MP_QSTR_ethereum_pubkeyhash), MP_ROM_PTR(&mod_trezorcrypto_HDNode_ethereum_pubkeyhash_obj) },
#endif
};
STATIC MP_DEFINE_CONST_DICT(mod_trezorcrypto_HDNode_locals_dict, mod_trezorcrypto_HDNode_locals_dict_table);

STATIC const mp_obj_type_t mod_trezorcrypto_HDNode_type = {
    { &mp_type_type },
    .name = MP_QSTR_HDNode,
    .make_new = mod_trezorcrypto_HDNode_make_new,
    .locals_dict = (void*)&mod_trezorcrypto_HDNode_locals_dict,
};

/// def deserialize(self, value: str, version_public: int, version_private: int) -> HDNode:
///     '''
///     Construct a BIP0032 HD node from a base58-serialized value.
///     '''
STATIC mp_obj_t mod_trezorcrypto_bip32_deserialize(mp_obj_t value, mp_obj_t version_public, mp_obj_t version_private) {
    mp_buffer_info_t valueb;
    mp_get_buffer_raise(value, &valueb, MP_BUFFER_READ);
    if (valueb.len == 0) {
        mp_raise_ValueError("Invalid value");
    }
    uint32_t vpub = mp_obj_get_int_truncated(version_public);
    uint32_t vpriv = mp_obj_get_int_truncated(version_private);
    HDNode hdnode;
    uint32_t fingerprint;
    if (hdnode_deserialize(valueb.buf, vpub, vpriv, SECP256K1_NAME, &hdnode, &fingerprint) < 0) {
        mp_raise_ValueError("Failed to deserialize");
    }

    mp_obj_HDNode_t *o = m_new_obj(mp_obj_HDNode_t);
    o->base.type = &mod_trezorcrypto_HDNode_type;
    o->hdnode = hdnode;
    o->fingerprint = fingerprint;

    o->have_private = false;
    for(int j=0; j<32; j++) {
        if(hdnode.private_key[j] != 0) {
            o->have_private = true;
            break;
        }
    }

    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_trezorcrypto_bip32_deserialize_obj, mod_trezorcrypto_bip32_deserialize);

/// def from_seed(seed: bytes, curve_name: str) -> HDNode:
///     '''
///     Construct a BIP0032 HD node from a BIP0039 seed value.
///     '''
STATIC mp_obj_t mod_trezorcrypto_bip32_from_seed(mp_obj_t seed, mp_obj_t curve_name) {
    mp_buffer_info_t seedb;
    mp_get_buffer_raise(seed, &seedb, MP_BUFFER_READ);
    if (seedb.len == 0) {
        mp_raise_ValueError("Invalid seed");
    }
    mp_buffer_info_t curveb;
    mp_get_buffer_raise(curve_name, &curveb, MP_BUFFER_READ);
    if (curveb.len == 0) {
        mp_raise_ValueError("Invalid curve name");
    }
    HDNode hdnode;
    if (!hdnode_from_seed(seedb.buf, seedb.len, curveb.buf, &hdnode)) {
        mp_raise_ValueError("Failed to derive the root node");
    }
    mp_obj_HDNode_t *o = m_new_obj(mp_obj_HDNode_t);
    o->base.type = &mod_trezorcrypto_HDNode_type;
    o->hdnode = hdnode;
    o->have_private = true;
    return MP_OBJ_FROM_PTR(o);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_trezorcrypto_bip32_from_seed_obj, mod_trezorcrypto_bip32_from_seed);

STATIC const mp_rom_map_elem_t mod_trezorcrypto_bip32_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_bip32) },
    { MP_ROM_QSTR(MP_QSTR_HDNode), MP_ROM_PTR(&mod_trezorcrypto_HDNode_type) },
    { MP_ROM_QSTR(MP_QSTR_deserialize), MP_ROM_PTR(&mod_trezorcrypto_bip32_deserialize_obj) },
    { MP_ROM_QSTR(MP_QSTR_from_seed), MP_ROM_PTR(&mod_trezorcrypto_bip32_from_seed_obj) },
};
STATIC MP_DEFINE_CONST_DICT(mod_trezorcrypto_bip32_globals, mod_trezorcrypto_bip32_globals_table);

STATIC const mp_obj_module_t mod_trezorcrypto_bip32_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mod_trezorcrypto_bip32_globals,
};
