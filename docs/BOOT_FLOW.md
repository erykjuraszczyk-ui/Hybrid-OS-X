# Boot Flow — 0.1.0

1. UEFI firmware loads \EFI\BOOT\BOOTX64.EFI as an x86-64 EFI application.
2. efi_main initializes GNU-EFI support and the COM1 diagnostic port.
3. The boot phase discovers the Graphics Output Protocol and records framebuffer metadata when available.
4. The boot phase obtains the UEFI memory-map buffer size, allocates a buffer, and captures the final map immediately before exit.
5. ExitBootServices(image_handle, map_key) is attempted. EFI_INVALID_PARAMETER causes a bounded memory-map refresh and retry; other errors halt the machine.
6. After a successful exit, no UEFI Boot Services or firmware console functions are called.
7. kernel_main validates the versioned boot-info structure, prints diagnostics, summarizes descriptors, draws a basic framebuffer screen when supported, then halts.

## Current execution assumptions

The image runs in the x86-64 environment provided by UEFI. This milestone does not install its own GDT/IDT, create new page tables, switch to a kernel-owned stack, or establish a separate kernel ELF loader. Maskable interrupts are disabled before kernel initialization. The next boot milestone must formalize these transitions and add exception handling before enabling interrupts.
