.RECIPEPREFIX = >
ifndef PS5_PAYLOAD_SDK
$(error PS5_PAYLOAD_SDK is not set)
endif
include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk

ELF := Ps5Downloadly.elf

all: $(ELF)

$(ELF): main.c
> $(CC) -Wall -O2 main.c -lSceNet -lSceSsl -lSceHttp2 -o $@

clean:
> rm -f $(ELF)
