# Bitcoin and Crypto Algorithms for Micropython

## modcryptocurrency

This project aims to take the Micropython integration from
`.../embed/extmod/modtrezorcrypto`
and make it useful to other embedded cryptocurrency projects.

Changes:

- shorten the module name to something easier to type
- fix minor build issues to track micropython changes

# Credits

This code originates from an [Open Source Trezor project](https://github.com/trezor/trezor-core/tree/master/embed/extmod/modtrezorcrypto).

# Licence

GPL v3 like the original material.


# Change Log

- results from `bip39_complete_word()` could not distinguish that "act" is both
  a prefix and actual word in the wordlist.

