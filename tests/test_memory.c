#include "hybrid/boot_info.h"
#include "memory.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct test_descriptor {
    hybrid_uefi_memory_descriptor_t descriptor;
    uint64_t extension;
} test_descriptor_t;

static void test_memory_summary_respects_descriptor_stride(void)
{
    test_descriptor_t descriptors[3];
    hybrid_boot_info_t boot_info;
    hybrid_memory_summary_t summary;

    memset(descriptors, 0, sizeof(descriptors));
    memset(&boot_info, 0, sizeof(boot_info));

    descriptors[0].descriptor.type = 7U;
    descriptors[0].descriptor.physical_start = 0x100000U;
    descriptors[0].descriptor.number_of_pages = 256U;

    descriptors[1].descriptor.type = 4U;
    descriptors[1].descriptor.physical_start = 0x200000U;
    descriptors[1].descriptor.number_of_pages = 128U;

    descriptors[2].descriptor.type = 0U;
    descriptors[2].descriptor.physical_start = 0x300000U;
    descriptors[2].descriptor.number_of_pages = 16U;

    boot_info.magic = HYBRID_BOOT_INFO_MAGIC;
    boot_info.flags = HYBRID_BOOT_FLAG_MEMORY_MAP;
    boot_info.memory_map = (uint64_t)(uintptr_t)descriptors;
    boot_info.memory_map_size = sizeof(descriptors);
    boot_info.memory_descriptor_size = (uint32_t)sizeof(test_descriptor_t);

    assert(hybrid_memory_summarize(&boot_info, &summary) == 1);
    assert(summary.descriptor_count == 3U);
    assert(summary.conventional_pages == 256U);
    assert(summary.reclaimable_boot_services_pages == 128U);
    assert(summary.malformed_descriptors == 0U);
}

static void test_memory_summary_rejects_invalid_handoff(void)
{
    hybrid_boot_info_t boot_info;
    hybrid_memory_summary_t summary;

    memset(&boot_info, 0, sizeof(boot_info));
    assert(hybrid_memory_summarize(&boot_info, &summary) == 0);
}

static void test_memory_summary_detects_overflowing_region(void)
{
    hybrid_uefi_memory_descriptor_t descriptor;
    hybrid_boot_info_t boot_info;
    hybrid_memory_summary_t summary;

    memset(&descriptor, 0, sizeof(descriptor));
    memset(&boot_info, 0, sizeof(boot_info));
    descriptor.type = 7U;
    descriptor.physical_start = UINT64_MAX - 0x1000U;
    descriptor.number_of_pages = 4U;

    boot_info.magic = HYBRID_BOOT_INFO_MAGIC;
    boot_info.flags = HYBRID_BOOT_FLAG_MEMORY_MAP;
    boot_info.memory_map = (uint64_t)(uintptr_t)&descriptor;
    boot_info.memory_map_size = sizeof(descriptor);
    boot_info.memory_descriptor_size = sizeof(descriptor);

    assert(hybrid_memory_summarize(&boot_info, &summary) == 1);
    assert(summary.descriptor_count == 1U);
    assert(summary.malformed_descriptors == 1U);
    assert(summary.conventional_pages == 0U);
}

int main(void)
{
    test_memory_summary_respects_descriptor_stride();
    test_memory_summary_rejects_invalid_handoff();
    test_memory_summary_detects_overflowing_region();
    puts("HYBRID OS X host tests: PASS");
    return 0;
}
