#ifndef HYBRID_MEMORY_H
#define HYBRID_MEMORY_H

#include <stdint.h>
#include "hybrid/boot_info.h"

typedef struct hybrid_memory_summary {
    uint64_t conventional_pages;
    uint64_t reclaimable_boot_services_pages;
    uint64_t descriptor_count;
    uint64_t malformed_descriptors;
} hybrid_memory_summary_t;

int hybrid_memory_summarize(const hybrid_boot_info_t *boot_info,
                            hybrid_memory_summary_t *summary);

#endif
