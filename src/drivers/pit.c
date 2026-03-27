#include "pit.h"
#include "common.h"

void pit_init(u32int frequency) {
    u32int divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}
