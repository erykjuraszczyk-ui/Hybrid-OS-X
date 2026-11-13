# HYBRID OS X

HYBRID OS X is an experimental native operating-system project for x86-64 PCs. The project establishes a UEFI boot path, a freestanding C17 kernel entry point, serial diagnostics, a GOP framebuffer boot display, a read-only UEFI memory-map view, and initial x86-64 IDT exception gates (vectors 0-31).

**Status: 0.2.0 kernel-core preview (incremental).** This is not yet a general-purpose operating system. Process isolation, scheduling, virtual memory, filesystems, networking, userspace, and a complete driver model are not implemented.

## Technical baseline

| Area | Baseline |
|---|---|
| CPU architecture | x86-64 |
| Firmware | UEFI x64 |
| Language | ISO C17, freestanding |
| EFI toolchain | GCC + GNU-EFI + GNU binutils |
| Emulator | QEMU q35 + OVMF |
| Kernel model | Modular monolithic kernel |

The boot application and initial kernel are linked into one EFI executable for this first milestone. This avoids claiming that a separate ELF loader, page-table handoff, or independent kernel image already exists.

## Build on Ubuntu

    sudo apt update
    sudo apt install -y build-essential gcc binutils make gnu-efi qemu-system-x86 ovmf
    make clean all
    make test
    make run

The EFI application is produced at build/esp/EFI/BOOT/BOOTX64.EFI. QEMU sends kernel diagnostics to the terminal over COM1.

If OVMF is installed elsewhere, use:

    make run OVMF_CODE=/path/to/OVMF_CODE.fd OVMF_VARS=/path/to/OVMF_VARS.fd

## Targets

- make all — build BOOTX64.EFI and prepare the EFI system partition.
- make test — run host-side tests for UEFI memory-map parsing and bounds checks.
- make check — verify the generated EFI application and its PE/COFF signature.
- make run — boot in QEMU/OVMF.
- make clean — remove generated build files.

## Repository map

- boot/uefi/ — UEFI entry point and ExitBootServices handoff.
- include/hybrid/ — versioned boot contract.
- kernel/arch/x86_64/ — CPU primitives, IDT and exception entry stubs.
- kernel/console/ — GOP boot display.
- kernel/core/ — kernel entry and panic handling.
- kernel/drivers/serial/ — COM1 diagnostics.
- kernel/mm/ — UEFI memory-map inspection.
- docs/ — architecture, boot flow, security model and roadmap.
- scripts/ — toolchain and emulator helpers.

## Engineering rules

1. Keep architecture-specific instructions under kernel/arch/<architecture>/.
2. Confine UEFI Boot Services calls to the boot phase. No Boot Services call is permitted after a successful ExitBootServices.
3. Validate boot handoff fields before use.
4. Treat firmware memory-map data as external input; bounds-check every descriptor walk.
5. Keep cryptographic key management and PKI out of the kernel. They belong behind separately reviewed service boundaries.
6. Never commit generated binaries, firmware images, private keys, signing keys or secrets.

## Current limitations

The first milestone remains in UEFI-provided long mode and uses the firmware's initial address-space setup. It disables maskable interrupts before kernel initialization and halts on completion. It installs an initial IDT for CPU exception vectors 0-31, but does not yet install a kernel-owned GDT, reconfigure paging, initialize APs, reclaim boot-services memory, or provide a general-purpose allocator. GOP output supports the two common 32-bit RGB/BGR pixel formats; serial output is the reliable QEMU diagnostic path.

See docs/ARCHITECTURE.md and docs/ROADMAP.md for the current contract and staged plan.
