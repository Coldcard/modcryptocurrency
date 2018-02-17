#
# TODO: Make this useful... it should compile against microptyhon unix port or something
#

MPY_TOP ?= ../micropython
MP_CONFIGFILE ?= $(MPY_TOP)/ports/unix/mpconfigport.h

CFLAGS += -I$(MPY_TOP) -I$(MPY_TOP)/ports/unix/build -DMP_CONFIGFILE="\""$(MP_CONFIGFILE)"\""
CFLAGS += -DMICROPY_PY_TREZORCRYPTO=1 -Itrezor-crypto

# Include these files into your project.
C_FILES = crc.c modtcc.c

# and this includes lots of other stuff
# default target is here
modtcc.o: modtcc-*.c

all: $(OBJ_FILES)
	@echo syntax ok: $(OBJ_FILES)

OBJ_FILES = $(C_FILES:%.c=%.o)

TC_LIB = trezor-crypto/libtrezor-crypto.so

TARGET = example.o
$(TARGET): $(OBJ_FILES) $(TC_LIB) Makefile
	$(CC) -o $(TARGET) $(OBJ_FILES) $(TC_LIB)

clean:
	$(RM) *.o *.so *.a
