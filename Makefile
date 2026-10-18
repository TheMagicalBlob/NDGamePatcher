# NDGamePatcher (TheMagicalBlob)

ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK is undefined)
endif

ELF := dmenu_payload.bin
SENDER := ./ps.exe

CFLAGS := -Wall -Wno-unused-function -g -lSceSystemService


all: $(ELF)
asm: $(DBG)


$(DBG): main.c assertion.c memory.c proc.c dmenu.c
	$(info Outputting compiled source only)
	$(CC) -S $@ $^


$(ELF): main.c assertion.c memory.c proc.c dmenu.c
	$(info Removing and rebuilding $@)
	rm -f $(ELF)
	$(CC) $(CFLAGS) -o $@ $(realpath $^)
