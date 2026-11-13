#include "serial.h"

#include <stdint.h>

#define COM1_BASE 0x3F8U

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void hybrid_serial_initialize(void)
{
    outb(COM1_BASE + 1U, 0x00U);
    outb(COM1_BASE + 3U, 0x80U);
    outb(COM1_BASE + 0U, 0x01U);
    outb(COM1_BASE + 1U, 0x00U);
    outb(COM1_BASE + 3U, 0x03U);
    outb(COM1_BASE + 2U, 0xC7U);
    outb(COM1_BASE + 4U, 0x0BU);
}

void hybrid_serial_write_char(char value)
{
    unsigned int attempts = 100000U;
    while (attempts-- > 0U) {
        if ((inb(COM1_BASE + 5U) & 0x20U) != 0U) {
            outb(COM1_BASE, (uint8_t)value);
            return;
        }
    }
}

void hybrid_serial_write(const char *text)
{
    if (text == 0) {
        return;
    }
    while (*text != '\0') {
        if (*text == '\n') {
            hybrid_serial_write_char('\r');
        }
        hybrid_serial_write_char(*text++);
    }
}

void hybrid_serial_write_u64(unsigned long long value)
{
    char digits[21];
    unsigned int index = 0U;

    if (value == 0ULL) {
        hybrid_serial_write_char('0');
        return;
    }

    while (value > 0ULL && index < (unsigned int)sizeof(digits)) {
        digits[index++] = (char)('0' + (value % 10ULL));
        value /= 10ULL;
    }

    while (index > 0U) {
        hybrid_serial_write_char(digits[--index]);
    }
}
