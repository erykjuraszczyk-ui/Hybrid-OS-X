#include "panic.h"

#include "../arch/x86_64/cpu.h"
#include "../drivers/serial/serial.h"

_Noreturn void hybrid_panic(const char *reason)
{
    hybrid_cpu_disable_interrupts();
    hybrid_serial_write("\n[HYBRID][PANIC] ");
    hybrid_serial_write(reason != 0 ? reason : "unspecified failure");
    hybrid_serial_write("\nSystem halted.\n");
    hybrid_cpu_halt_forever();
}
