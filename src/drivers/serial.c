// serial.c
#include "serial.h"
#include "common.h"
#include "irq.h"
#include "keyboard_buffer.h"
#include "input.h"
#include "monitor.h"

#define COM1 0x3F8

static int serial_is_transmit_empty(void) {
    return inb(COM1 + 5) & 0x20;
}

static int serial_received(void) {
    return inb(COM1 + 5) & 0x01;
}

static void serial_irq_handler(struct registers* regs) {
    (void)regs;
    while (serial_received()) {
        char c = (char)inb(COM1);
        if (c) {
            kbd_buffer_push(c);
            input_push_key(c);
        }
    }
}

void serial_init(void) {
    outb(COM1 + 1, 0x00);    // Disable all interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB
    outb(COM1 + 0, 0x03);    // Baud rate 38400 (divisor 3)
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // Enable FIFO, clear, 14-byte threshold
    outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void serial_write_char(char c) {
    while (serial_is_transmit_empty() == 0) { }
    outb(COM1, (u8int)c);
}

void serial_write(const char *s) {
    if (!s) return;
    while (*s) {
        if (*s == '\n') {
            serial_write_char('\r');
        }
        serial_write_char(*s++);
    }
}

int serial_read_char(void) {
    if (!serial_received()) {
        return -1;
    }
    return (int)inb(COM1);
}

void serial_enable_rx_interrupts(void) {
    register_irq_handler(4, serial_irq_handler);
    outb(COM1 + 1, 0x01);    // Enable received data available interrupt
    outb(COM1 + 4, 0x0B);    // Ensure IRQs enabled (OUT2)
}
