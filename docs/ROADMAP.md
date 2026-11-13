# HYBRID OS X — Engineering Roadmap

A component is marked implemented only when code exists and a reproducible test covers it.

## M0 — UEFI bootstrap (0.1.0)

- [x] x86-64 UEFI application entry.
- [x] Versioned boot-info contract.
- [x] UEFI memory-map capture and bounded ExitBootServices retry.
- [x] COM1 serial diagnostics.
- [x] Minimal GOP boot display for common RGB/BGR 32-bit modes.
- [x] Read-only UEFI memory-map summary.
- [x] Make-based build, QEMU runner, host-side memory-map tests and CI build check.

## M1 — Stable CPU entry and diagnostics (0.2.x)

- [ ] Dedicated assembly entry and kernel-owned stack.
- [ ] GDT, IDT and exception stubs.
- [ ] Fatal exception reporting over serial and framebuffer.
- [ ] Linker script and explicit kernel memory layout.
- [ ] QEMU smoke tests that verify serial boot markers.

## M2 — Memory and execution core (0.3.x)

- [ ] Physical frame allocator with reserved-region tracking.
- [ ] Kernel-owned page tables and virtual-memory mapping API.
- [ ] Heap allocator with guardrails and diagnostics.
- [ ] APIC timer and interrupt-controller abstraction.
- [ ] SMP discovery and secondary CPU bring-up.

## M3 — Kernel services (0.4.x)

- [ ] Thread model, context switching and scheduler.
- [ ] Synchronization primitives and lock-order policy.
- [ ] IPC primitives and capability-oriented handles.
- [ ] Kernel object and driver lifecycle model.

## M4 — Storage and userspace (0.5.x)

- [ ] PCI enumeration and block-device interface.
- [ ] GPT parser and read-only FAT driver for boot media.
- [ ] VFS abstraction and first writable filesystem.
- [ ] ELF64 userspace loader, syscall ABI and init process.

## M5 — Security platform (0.6.x onward)

- [ ] Secure Boot verification and measured-boot design.
- [ ] Privilege separation, capabilities and audit events.
- [ ] PKI service with explicit trust anchors, lifecycle and recovery.
- [ ] Privacy/data governance, retention and deletion controls.
- [ ] Developer integration SDK, signed updates and release governance.
- [ ] Operations telemetry, incident-response procedures and reproducible builds.

The accepted SPEC-003 PKI decisions remain governance inputs for the security-platform stage. They are not reinterpreted as permission to place cryptographic key management inside the kernel.
