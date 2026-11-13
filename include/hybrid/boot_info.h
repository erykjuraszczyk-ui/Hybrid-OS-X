#ifndef HYBRID_BOOT_INFO_H
#define HYBRID_BOOT_INFO_H

#include <stdint.h>

#define HYBRID_BOOT_INFO_MAGIC UINT64_C(0x4859425249444F53) /* HYBRIDOS */
#define HYBRID_BOOT_INFO_VERSION 1U

enum hybrid_boot_flags {
    HYBRID_BOOT_FLAG_FRAMEBUFFER = 1U << 0,
    HYBRID_BOOT_FLAG_MEMORY_MAP  = 1U << 1
};

/* Layout matches the 40-byte UEFI EFI_MEMORY_DESCRIPTOR prefix. */
typedef struct hybrid_uefi_memory_descriptor {
    uint32_t type;
    uint32_t pad;
    uint64_t physical_start;
    uint64_t virtual_start;
    uint64_t number_of_pages;
    uint64_t attribute;
} hybrid_uefi_memory_descriptor_t;

typedef struct hybrid_framebuffer_info {
    uint64_t base;
    uint64_t size_bytes;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scanline;
    uint32_t pixel_format;
} hybrid_framebuffer_info_t;

typedef struct hybrid_boot_info {
    uint64_t magic;
    uint32_t version;
    uint32_t struct_size;
    uint32_t flags;
    uint32_t memory_descriptor_size;
    uint32_t memory_descriptor_version;
    uint32_t reserved;
    uint64_t memory_map;
    uint64_t memory_map_size;
    hybrid_framebuffer_info_t framebuffer;
} hybrid_boot_info_t;

_Static_assert(sizeof(hybrid_uefi_memory_descriptor_t) == 40U,
               "UEFI memory descriptor prefix must remain 40 bytes");

#endif
