PROJECT := HYBRID-OS-X
BUILD_DIR := build
ESP_DIR := $(BUILD_DIR)/esp
EFI_APP := $(ESP_DIR)/EFI/BOOT/BOOTX64.EFI
EFI_SO := $(BUILD_DIR)/$(PROJECT).so
HOST_TEST := $(BUILD_DIR)/tests/test_memory

CC ?= gcc
LD ?= ld
OBJCOPY ?= objcopy
OVMF_CODE ?= $(firstword $(wildcard /usr/share/OVMF/OVMF_CODE.fd /usr/share/ovmf/OVMF_CODE.fd /usr/share/edk2/x64/OVMF_CODE.fd))
OVMF_VARS ?= $(firstword $(wildcard /usr/share/OVMF/OVMF_VARS.fd /usr/share/ovmf/OVMF_VARS.fd /usr/share/edk2/x64/OVMF_VARS.fd))

EFI_LIBDIR ?= $(if $(wildcard /usr/lib/crt0-efi-x86_64.o),/usr/lib,$(if $(wildcard /usr/lib64/crt0-efi-x86_64.o),/usr/lib64,$(if $(wildcard /usr/lib/gnuefi/crt0-efi-x86_64.o),/usr/lib/gnuefi,/usr/lib)))
EFI_INCLUDEDIR ?= /usr/include/efi
EFI_CRT ?= $(EFI_LIBDIR)/crt0-efi-x86_64.o
EFI_LDS ?= $(EFI_LIBDIR)/elf_x86_64_efi.lds

CPPFLAGS := -Iinclude -I$(EFI_INCLUDEDIR) -I$(EFI_INCLUDEDIR)/x86_64 -DEFI_FUNCTION_WRAPPER
CFLAGS := -std=c17 -O2 -Wall -Wextra -Werror -Wno-unused-parameter \
          -ffreestanding -fno-builtin -fno-stack-protector -fno-stack-check \
          -fpic -fshort-wchar -mno-red-zone -maccumulate-outgoing-args \
          -fno-unwind-tables -fno-asynchronous-unwind-tables
LDFLAGS := -nostdlib -znocombreloc -T $(EFI_LDS) -shared -Bsymbolic

SOURCES := \
    boot/uefi/entry.c \
    kernel/kernel.c \
    kernel/arch/x86_64/cpu.c \
    kernel/core/panic.c \
    kernel/drivers/serial/serial.c \
    kernel/mm/memory.c \
    kernel/console/framebuffer.c
OBJECTS := $(patsubst %.c,$(BUILD_DIR)/obj/%.o,$(SOURCES))

.PHONY: all clean check test run help

all: $(EFI_APP)

$(BUILD_DIR)/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(EFI_SO): $(OBJECTS) | $(BUILD_DIR)
	@test -f "$(EFI_CRT)" || (echo "GNU-EFI crt object not found: $(EFI_CRT)"; exit 1)
	@test -f "$(EFI_LDS)" || (echo "GNU-EFI linker script not found: $(EFI_LDS)"; exit 1)
	$(LD) $(LDFLAGS) $(EFI_CRT) $(OBJECTS) -L$(EFI_LIBDIR) -lgnuefi -lefi -o $@

$(EFI_APP): $(EFI_SO)
	@mkdir -p $(dir $@)
	$(OBJCOPY) \
		-j .text -j .sdata -j .data -j .rodata -j .dynamic \
		-j .dynsym -j .rel -j .rela -j .reloc \
		--output-target=efi-app-x86_64 $< $@

$(BUILD_DIR):
	@mkdir -p $@

check: all
	@test -s "$(EFI_APP)"
	@file "$(EFI_APP)" | grep -Eqi 'PE32[+].*EFI application|PE32[+] executable'
	@echo "HYBRID OS X EFI application verified: $(EFI_APP)"

test: $(HOST_TEST)
	$(HOST_TEST)

$(HOST_TEST): tests/test_memory.c kernel/mm/memory.c include/hybrid/boot_info.h kernel/mm/memory.h
	@mkdir -p $(dir $@)
	$(CC) -std=c17 -O2 -Wall -Wextra -Werror -Iinclude -Ikernel/mm \
		tests/test_memory.c kernel/mm/memory.c -o $@

run: check
	@test -n "$(OVMF_CODE)" || (echo "OVMF_CODE not found. Install ovmf or set OVMF_CODE=/path/to/OVMF_CODE.fd"; exit 1)
	@test -n "$(OVMF_VARS)" || (echo "OVMF_VARS not found. Install ovmf or set OVMF_VARS=/path/to/OVMF_VARS.fd"; exit 1)
	@test -f "$(BUILD_DIR)/OVMF_VARS.fd" || cp "$(OVMF_VARS)" "$(BUILD_DIR)/OVMF_VARS.fd"
	qemu-system-x86_64 \
		-machine q35 -m 512M \
		-drive if=pflash,format=raw,readonly=on,file="$(OVMF_CODE)" \
		-drive if=pflash,format=raw,file="$(BUILD_DIR)/OVMF_VARS.fd" \
		-drive format=raw,file=fat:rw:"$(ESP_DIR)" \
		-serial stdio -monitor none -no-reboot

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Targets: all (default), check, test, run, clean"
	@echo "Variables: EFI_LIBDIR, EFI_INCLUDEDIR, EFI_CRT, EFI_LDS, OVMF_CODE, OVMF_VARS"
