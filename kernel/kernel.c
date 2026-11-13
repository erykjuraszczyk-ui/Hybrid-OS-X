#include "hybrid/boot_info.h"
#include "arch/x86_64/cpu.h"\n#include "arch/x86_64/idt.h"
#include "console/framebuffer.h"
#include "core/panic.h"
#include "drivers/serial/serial.h"
#include "mm/memory.h"

#define HYBRID_VERSION "0.2.0"

_Noreturn void kernel_main(const hybrid_boot_info_t *boot_info)
{
    hybrid_memory_summary_t memory_summary;

    hybrid_cpu_disable_interrupts();
    hybrid_serial_initialize();
    hybrid_serial_write("\n========================================\n");
    hybrid_serial_write(" HYBRID OS X  |  native kernel bootstrap\n");
    hybrid_serial_write(" Version: " HYBRID_VERSION "\n");
    hybrid_serial_write(" Architecture: x86_64\n");
    hybrid_serial_write(" Language: C17\n");
    hybrid_serial_write("========================================\n");

    if (boot_info == 0 || boot_info->magic != HYBRID_BOOT_INFO_MAGIC ||
        boot_info->version != HYBRID_BOOT_INFO_VERSION ||
        boot_info->struct_size < sizeof(hybrid_boot_info_t)) {
        hybrid_panic("invalid or unsupported boot information block");
    }

    hybrid_serial_write("[BOOT] UEFI ExitBootServices handoff: OK\n");

    if (hybrid_memory_summarize(boot_info, &memory_summary) != 0) {
        hybrid_serial_write("[MM] UEFI memory descriptors: ");
        hybrid_serial_write_u64(memory_summary.descriptor_count);
        hybrid_serial_write("\n[MM] Conventional memory (MiB): ");
        hybrid_serial_write_u64(memory_summary.conventional_pages / 256ULL);
        hybrid_serial_write("\n[MM] Reclaimable boot-services memory (MiB): ");
        hybrid_serial_write_u64(memory_summary.reclaimable_boot_services_pages / 256ULL);
        hybrid_serial_write("\n");
    } else {
        hybrid_serial_write("[MM] Memory map unavailable or malformed.\n");
    }

    if ((boot_info->flags & HYBRID_BOOT_FLAG_FRAMEBUFFER) != 0U) {
        hybrid_framebuffer_draw_boot_screen(&boot_info->framebuffer);
        hybrid_serial_write("[GOP] Framebuffer boot screen: requested\n");
    } else {
        hybrid_serial_write("[GOP] Framebuffer unavailable; serial mode active.\n");
    }

    hybrid_serial_write("[CORE] Bootstrap complete. CPU halted safely.\n");
    hybrid_cpu_halt_forever();
}
