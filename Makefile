#
# TODO: Make this useful... it should compile against microptyhon unix port
#

MPY_TOP ?= ../micropython
MP_CONFIGFILE ?= $(MPY_TOP)/ports/unix/mpconfigport.h

CFLAGS += -I$(MPY_TOP) -I$(MPY_TOP)/ports/unix/build -DMP_CONFIGFILE="\""$(MP_CONFIGFILE)"\""
CFLAGS += -DMICROPY_PY_TREZORCRYPTO=1 -Itrezor-crypto

C_FILES = crc.c modtrezorcrypto.c

OBJ_FILES = $(C_FILES:%.c=%.o)

TARGET = example.o

all: $(TARGET)

TC_LIB = trezor-crypto/libtrezor-crypto.so

$(TARGET): $(OBJ_FILES) $(TC_LIB) Makefile
	$(CC) -o $(TARGET) $(OBJ_FILES) $(TC_LIB)

clean:
	$(RM) *.o *.so *.a
