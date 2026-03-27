// serial.h
#ifndef SERIAL_H
#define SERIAL_H

void serial_init(void);
void serial_write_char(char c);
void serial_write(const char *s);
int serial_read_char(void);
void serial_enable_rx_interrupts(void);

#endif
