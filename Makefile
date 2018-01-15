
MPY_TOP ?= ../micropython
MP_CONFIGFILE ?= $(MPY_TOP)/ports/unix/mpconfigport.h

CFLAGS += -I$(MPY_TOP) -I$(MPY_TOP)/ports/unix/build -DMP_CONFIGFILE="\""$(MP_CONFIGFILE)"\"" -DMICROPY_PY_TREZORCRYPTO=1 -Itrezor-crypto

C_FILES = crc.c modtrezorcrypto.c

OBJ_FILES = $(C_FILES:%.c=%.o)

all: $(OBJ_FILES) Makefile

clean:
	$(RM) *.o *.so *.a
