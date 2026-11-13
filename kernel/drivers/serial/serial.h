#ifndef HYBRID_SERIAL_H
#define HYBRID_SERIAL_H

void hybrid_serial_initialize(void);
void hybrid_serial_write_char(char value);
void hybrid_serial_write(const char *text);
void hybrid_serial_write_u64(unsigned long long value);

#endif
