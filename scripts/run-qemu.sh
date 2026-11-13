#!/usr/bin/env sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT_DIR"

if [ ! -f build/esp/EFI/BOOT/BOOTX64.EFI ]; then
    make all
fi

OVMF_CODE=$(printenv OVMF_CODE || true)
OVMF_VARS=$(printenv OVMF_VARS || true)
if [ -z "$OVMF_CODE" ]; then
    for candidate in \
        /usr/share/OVMF/OVMF_CODE.fd \
        /usr/share/ovmf/OVMF_CODE.fd \
        /usr/share/edk2/x64/OVMF_CODE.fd; do
        if [ -f "$candidate" ]; then
            OVMF_CODE=$candidate
            break
        fi
    done
fi

if [ -z "$OVMF_CODE" ] || [ ! -f "$OVMF_CODE" ]; then
    printf 'OVMF code image not found. Install ovmf or set OVMF_CODE.\n' >&2
    exit 1
fi

if [ -z "$OVMF_VARS" ]; then
    for candidate in \
        /usr/share/OVMF/OVMF_VARS.fd \
        /usr/share/ovmf/OVMF_VARS.fd \
        /usr/share/edk2/x64/OVMF_VARS.fd; do
        if [ -f "$candidate" ]; then
            OVMF_VARS=$candidate
            break
        fi
    done
fi

if [ -z "$OVMF_VARS" ] || [ ! -f "$OVMF_VARS" ]; then
    printf 'OVMF variable-store image not found. Install ovmf or set OVMF_VARS.\n' >&2
    exit 1
fi

if [ ! -f build/OVMF_VARS.fd ]; then
    cp "$OVMF_VARS" build/OVMF_VARS.fd
fi

exec qemu-system-x86_64 \
    -machine q35 -m 512M \
    -drive if=pflash,format=raw,readonly=on,file="$OVMF_CODE" \
    -drive if=pflash,format=raw,file=build/OVMF_VARS.fd \
    -drive format=raw,file=fat:rw:build/esp \
    -serial stdio -monitor none -no-reboot
