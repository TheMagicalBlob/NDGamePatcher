# NDGamePatcher (TheMagicalBlob)

ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK is undefined)
endif

ELF := dmenu_payload.bin
SENDER := ./ps.exe

CFLAGS := -Wall -g -lSceSystemService

all: $(ELF)

asm: $(DBG)


$(DBG): main.c memory.c dmenu.c proc.c
	$(info Outputting compiled source only)
	$(CC) -S $@ $^

$(ELF): main.c memory.c dmenu.c proc.c
	$(info Removing and rebuilding payload)
	rm -f $(ELF)
	$(CC) $(CFLAGS) -o $@ $^
