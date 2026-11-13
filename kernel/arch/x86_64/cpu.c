#include "cpu.h"

void hybrid_cpu_disable_interrupts(void)
{
    __asm__ volatile ("cli" : : : "memory");
}

_Noreturn void hybrid_cpu_halt_forever(void)
{
    for (;;) {
        __asm__ volatile ("hlt" : : : "memory");
    }
}
