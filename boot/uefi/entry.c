#include <efi.h>
#include <efilib.h>
#include <efiprot.h>

#include "hybrid/boot_info.h"
#include "../../kernel/drivers/serial/serial.h"
#include "../../kernel/arch/x86_64/cpu.h"

#define HYBRID_MEMORY_MAP_EXTRA_DESCRIPTORS 16U
#define HYBRID_EXIT_RETRIES 4U

static EFI_GUID hybrid_graphics_output_protocol_guid = {
    0x9042A9DEU, 0x23DCU, 0x4A38U,
    { 0x96U, 0xFBU, 0x7AU, 0xDEU, 0xD0U, 0x80U, 0x51U, 0x6AU }
};

extern _Noreturn void kernel_main(const hybrid_boot_info_t *boot_info);

static _Noreturn void boot_failure(const char *message)
{
    hybrid_serial_write("[UEFI][ERROR] ");
    hybrid_serial_write(message);
    hybrid_serial_write("\n");
    hybrid_cpu_disable_interrupts();
    hybrid_cpu_halt_forever();
}

static void collect_framebuffer(hybrid_boot_info_t *boot_info,
                                EFI_SYSTEM_TABLE *system_table)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics = NULL;
    EFI_STATUS status;

    status = uefi_call_wrapper(system_table->BootServices->LocateProtocol,
                               3, &hybrid_graphics_output_protocol_guid,
                               NULL, (VOID **)&graphics);
    if (EFI_ERROR(status) || graphics == NULL || graphics->Mode == NULL ||
        graphics->Mode->Info == NULL) {
        return;
    }

    boot_info->framebuffer.base = graphics->Mode->FrameBufferBase;
    boot_info->framebuffer.size_bytes = graphics->Mode->FrameBufferSize;
    boot_info->framebuffer.width = graphics->Mode->Info->HorizontalResolution;
    boot_info->framebuffer.height = graphics->Mode->Info->VerticalResolution;
    boot_info->framebuffer.pixels_per_scanline = graphics->Mode->Info->PixelsPerScanLine;
    boot_info->framebuffer.pixel_format = (uint32_t)graphics->Mode->Info->PixelFormat;

    if (boot_info->framebuffer.base != 0U &&
        boot_info->framebuffer.width > 0U &&
        boot_info->framebuffer.height > 0U) {
        boot_info->flags |= HYBRID_BOOT_FLAG_FRAMEBUFFER;
    }
}

static void *collect_memory_map(hybrid_boot_info_t *boot_info,
                                EFI_SYSTEM_TABLE *system_table,
                                UINTN *allocated_size)
{
    EFI_STATUS status;
    UINTN map_size = 0U;
    UINTN map_key = 0U;
    UINTN descriptor_size = 0U;
    UINT32 descriptor_version = 0U;
    VOID *memory_map = NULL;

    status = uefi_call_wrapper(system_table->BootServices->GetMemoryMap,
                               5, &map_size, NULL, &map_key,
                               &descriptor_size, &descriptor_version);
    if (status != EFI_BUFFER_TOO_SMALL || descriptor_size == 0U) {
        return NULL;
    }

    map_size += descriptor_size * HYBRID_MEMORY_MAP_EXTRA_DESCRIPTORS;
    status = uefi_call_wrapper(system_table->BootServices->AllocatePool,
                               3, EfiLoaderData, map_size, &memory_map);
    if (EFI_ERROR(status) || memory_map == NULL) {
        return NULL;
    }

    *allocated_size = map_size;
    boot_info->memory_descriptor_size = (uint32_t)descriptor_size;
    boot_info->memory_descriptor_version = descriptor_version;
    return memory_map;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle,
                           EFI_SYSTEM_TABLE *system_table)
{
    EFI_STATUS status;
    hybrid_boot_info_t boot_info;
    VOID *memory_map;
    UINTN allocated_map_size = 0U;
    UINTN map_size;
    UINTN map_key = 0U;
    UINTN descriptor_size = 0U;
    UINT32 descriptor_version = 0U;
    unsigned int attempt;

    InitializeLib(image_handle, system_table);
    hybrid_serial_initialize();
    hybrid_serial_write("[UEFI] HYBRID OS X boot application started.\n");

    boot_info.magic = HYBRID_BOOT_INFO_MAGIC;
    boot_info.version = HYBRID_BOOT_INFO_VERSION;
    boot_info.struct_size = (uint32_t)sizeof(boot_info);
    boot_info.flags = 0U;
    boot_info.memory_descriptor_size = 0U;
    boot_info.memory_descriptor_version = 0U;
    boot_info.reserved = 0U;
    boot_info.memory_map = 0U;
    boot_info.memory_map_size = 0U;
    boot_info.framebuffer.base = 0U;
    boot_info.framebuffer.size_bytes = 0U;
    boot_info.framebuffer.width = 0U;
    boot_info.framebuffer.height = 0U;
    boot_info.framebuffer.pixels_per_scanline = 0U;
    boot_info.framebuffer.pixel_format = 0U;

    collect_framebuffer(&boot_info, system_table);
    memory_map = collect_memory_map(&boot_info, system_table, &allocated_map_size);
    if (memory_map == NULL) {
        boot_failure("could not allocate a UEFI memory-map buffer");
    }

    for (attempt = 0U; attempt < HYBRID_EXIT_RETRIES; ++attempt) {
        map_size = allocated_map_size;
        status = uefi_call_wrapper(system_table->BootServices->GetMemoryMap,
                                   5, &map_size, memory_map, &map_key,
                                   &descriptor_size, &descriptor_version);
        if (EFI_ERROR(status)) {
            boot_failure("GetMemoryMap failed before ExitBootServices");
        }

        boot_info.memory_map = (uint64_t)(uintptr_t)memory_map;
        boot_info.memory_map_size = (uint64_t)map_size;
        boot_info.memory_descriptor_size = (uint32_t)descriptor_size;
        boot_info.memory_descriptor_version = descriptor_version;
        boot_info.flags |= HYBRID_BOOT_FLAG_MEMORY_MAP;

        status = uefi_call_wrapper(system_table->BootServices->ExitBootServices,
                                   2, image_handle, map_key);
        if (!EFI_ERROR(status)) {
            break;
        }
        if (status != EFI_INVALID_PARAMETER) {
            boot_failure("ExitBootServices failed with an unexpected status");
        }
    }

    if (EFI_ERROR(status)) {
        boot_failure("ExitBootServices retry limit reached");
    }

    /* Firmware Boot Services are no longer available beyond this point. */
    kernel_main(&boot_info);
}
