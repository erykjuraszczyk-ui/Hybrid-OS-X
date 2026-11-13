# HYBRID OS X — Architecture Contract (bootstrap baseline)

## System shape

HYBRID OS X starts as a **modular monolithic kernel**. Kernel subsystems are separate compilation units with explicit headers and ownership boundaries, but they execute in one privileged address space. A microkernel/service split is not assumed before evidence justifies its isolation and IPC costs.

## Supported baseline

- ISA: AMD64 / x86-64.
- Firmware: UEFI x64.
- Language: ISO C17 plus narrowly scoped x86-64 inline assembly.
- Binary format: PE32+ EFI application.
- Build environment: GCC, GNU-EFI, GNU binutils and Make.
- Test platform: QEMU q35 with OVMF.

## Boot and ownership boundaries

boot/uefi/entry.c is the only component allowed to call UEFI Boot Services. It discovers GOP, captures a bounded memory-map snapshot, allocates the handoff buffer, and calls ExitBootServices. After a successful call, firmware Boot Services are unavailable. Control is passed to kernel_main using a versioned hybrid_boot_info_t contract.

For milestone 0.1.0, boot and kernel code are part of one EFI image. The kernel continues in the firmware-provided x86-64 execution mode and address space. The next boot milestone must define an architecture entry contract before introducing a separately linked kernel image.

## Subsystem boundaries

| Subsystem | Responsibility | State |
|---|---|---|
| UEFI boot | Firmware discovery, memory map and handoff | Initial implementation |
| x86-64 CPU | Interrupt disable and halt primitive | Initial implementation |
| Serial driver | COM1 diagnostics | Initial implementation |
| GOP console | Basic 32-bit RGB/BGR boot-screen drawing | Initial implementation |
| Memory | Read-only UEFI descriptor summary | Initial implementation |
| Interrupts | IDT, exception and IRQ dispatch | Planned |
| Memory manager | Physical allocator, paging and virtual memory | Planned |
| Scheduler | Threads and scheduling policy | Planned |
| IPC | Kernel messaging and synchronization | Planned |
| Storage | Block layer, partitioning and filesystem | Planned |
| Security services | Identity, trust, PKI and policy services | Governance track; not in kernel |

## Cross-platform policy

Architecture-specific code belongs under kernel/arch/<architecture>/. Shared kernel code must not depend on x86 port I/O or x86-only control registers. The first implementation target is x86-64; future architectures require separate boot and CPU backends rather than scattered conditional compilation.

## Reliability rules

- Boot data is versioned and validated before consumption.
- Memory-map walks use the supplied descriptor stride, not an assumed packed layout.
- Address and size calculations are checked for overflow.
- Firmware services are forbidden after ExitBootServices.
- Panic handling is deterministic and does not depend on heap allocation.
- Drivers expose bounded interfaces and explicit ownership.

## Explicit non-goals for 0.1.0

This release does not provide user accounts, Secure Boot signing, PKI enrollment, encrypted storage, networking, USB, ACPI power management, process isolation, a userspace ABI, or Windows/Linux application compatibility. Those require separate specifications, threat models and validation.
