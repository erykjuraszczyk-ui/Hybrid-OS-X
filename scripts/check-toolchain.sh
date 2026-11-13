#!/usr/bin/env sh
set -eu

missing=0
for command in gcc ld objcopy make; do
    if ! command -v "$command" >/dev/null 2>&1; then
        printf 'Missing command: %s\n' "$command" >&2
        missing=1
    fi
done

for path in /usr/include/efi/efi.h /usr/include/efi/efilib.h; do
    if [ ! -f "$path" ]; then
        printf 'Missing GNU-EFI header: %s\n' "$path" >&2
        missing=1
    fi
done

if [ "$missing" -ne 0 ]; then
    printf '\nInstall prerequisites on Ubuntu: sudo apt install build-essential gcc binutils make gnu-efi qemu-system-x86 ovmf\n' >&2
    exit 1
fi

printf 'Toolchain prerequisites found.\n'
