#include "idt.h"

#include <stddef.h>
#include <stdint.h>

#include "../../drivers/serial/serial.h"
#include "cpu.h"
#include "../../core/panic.h"

#define HYBRID_IDT_ENTRIES 256U
#define HYBRID_CPU_EXCEPTION_COUNT 32U
#define HYBRID_IDT_INTERRUPT_GATE 0x8EU

typedef struct __attribute__((packed)) hybrid_idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
} hybrid_idt_entry_t;

typedef struct __attribute__((packed)) hybrid_idtr {
    uint16_t limit;
    uint64_t base;
} hybrid_idtr_t;

_Static_assert(sizeof(hybrid_idt_entry_t) == 16U, "IDT entry must be 16 bytes");
_Static_assert(sizeof(hybrid_idtr_t) == 10U, "IDTR must be 10 bytes");

static hybrid_idt_entry_t hybrid_idt[HYBRID_IDT_ENTRIES]
    __attribute__((aligned(16)));

extern void (*hybrid_isr_stub_table[HYBRID_CPU_EXCEPTION_COUNT])(void);

static uint16_t hybrid_read_cs(void)
{
    uint16_t selector;
    __asm__ volatile ("mov %%cs, %0" : "=r"(selector));
    return selector;
}

static void hybrid_idt_set_gate(uint8_t vector, uintptr_t handler,
                                uint16_t selector)
{
    hybrid_idt_entry_t *entry = &hybrid_idt[vector];

    entry->offset_low = (uint16_t)(handler & UINT64_C(0xFFFF));
    entry->selector = selector;
    entry->ist = 0U;
    entry->attributes = HYBRID_IDT_INTERRUPT_GATE;
    entry->offset_middle = (uint16_t)((handler >> 16U) & UINT64_C(0xFFFF));
    entry->offset_high = (uint32_t)((handler >> 32U) & UINT64_C(0xFFFFFFFF));
    entry->reserved = 0U;
}

void hybrid_idt_initialize(void)
{
    hybrid_idtr_t descriptor;
    uint16_t selector = hybrid_read_cs();
    size_t index;

    for (index = 0U; index < HYBRID_IDT_ENTRIES; ++index) {
        hybrid_idt[index] = (hybrid_idt_entry_t){0};
    }

    for (index = 0U; index < HYBRID_CPU_EXCEPTION_COUNT; ++index) {
        hybrid_idt_set_gate((uint8_t)index,
                            (uintptr_t)hybrid_isr_stub_table[index],
                            selector);
    }

    descriptor.limit = (uint16_t)(sizeof(hybrid_idt) - 1U);
    descriptor.base = (uint64_t)(uintptr_t)&hybrid_idt[0];
    __asm__ volatile ("lidt %0" : : "m"(descriptor) : "memory");
}

_Noreturn void hybrid_exception_dispatch(uint64_t vector, uint64_t error_code)
{
    hybrid_cpu_disable_interrupts();
    hybrid_serial_write("\n[CPU][EXCEPTION] Vector: ");
    hybrid_serial_write_u64((unsigned long long)vector);
    hybrid_serial_write("\n[CPU][EXCEPTION] Error code: ");
    hybrid_serial_write_u64((unsigned long long)error_code);
    hybrid_serial_write("\n");
    hybrid_panic("unhandled CPU exception");
}
