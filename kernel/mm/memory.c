#include "memory.h"

#include <stddef.h>
#include <stdint.h>

#define HYBRID_UEFI_MEMORY_BOOT_SERVICES_CODE 3U
#define HYBRID_UEFI_MEMORY_BOOT_SERVICES_DATA 4U
#define HYBRID_UEFI_MEMORY_CONVENTIONAL       7U

static uint64_t add_saturating(uint64_t left, uint64_t right)
{
    if (UINT64_MAX - left < right) {
        return UINT64_MAX;
    }
    return left + right;
}

int hybrid_memory_summarize(const hybrid_boot_info_t *boot_info,
                            hybrid_memory_summary_t *summary)
{
    const uint8_t *map_bytes;
    uint64_t descriptor_index;
    uint64_t descriptor_count;

    if (boot_info == NULL || summary == NULL ||
        boot_info->magic != HYBRID_BOOT_INFO_MAGIC ||
        (boot_info->flags & HYBRID_BOOT_FLAG_MEMORY_MAP) == 0U ||
        boot_info->memory_map == 0U ||
        boot_info->memory_descriptor_size < sizeof(hybrid_uefi_memory_descriptor_t) ||
        boot_info->memory_map_size < boot_info->memory_descriptor_size) {
        return 0;
    }

    summary->conventional_pages = 0U;
    summary->reclaimable_boot_services_pages = 0U;
    summary->descriptor_count = 0U;
    summary->malformed_descriptors = 0U;

    map_bytes = (const uint8_t *)(uintptr_t)boot_info->memory_map;
    descriptor_count = boot_info->memory_map_size / boot_info->memory_descriptor_size;
    for (descriptor_index = 0U; descriptor_index < descriptor_count; ++descriptor_index) {
        uint64_t offset = descriptor_index * boot_info->memory_descriptor_size;
        const hybrid_uefi_memory_descriptor_t *descriptor =
            (const hybrid_uefi_memory_descriptor_t *)(const void *)(map_bytes + offset);

        summary->descriptor_count++;
        if (descriptor->number_of_pages == 0U ||
            descriptor->number_of_pages >
                (UINT64_MAX - descriptor->physical_start) / UINT64_C(4096)) {
            summary->malformed_descriptors++;
            continue;
        }

        if (descriptor->type == HYBRID_UEFI_MEMORY_CONVENTIONAL) {
            summary->conventional_pages = add_saturating(
                summary->conventional_pages, descriptor->number_of_pages);
        } else if (descriptor->type == HYBRID_UEFI_MEMORY_BOOT_SERVICES_CODE ||
                   descriptor->type == HYBRID_UEFI_MEMORY_BOOT_SERVICES_DATA) {
            summary->reclaimable_boot_services_pages = add_saturating(
                summary->reclaimable_boot_services_pages,
                descriptor->number_of_pages);
        }
    }

    return summary->descriptor_count > 0U;
}
