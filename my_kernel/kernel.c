#include "idt.h"
#include "gdt.h"

extern void monitor_write(char *text);

void kernel_main() {
    init_gdt();
    init_idt();

    monitor_write("Learning-OS kernel baslatildi.\n");
    monitor_write("IRQ0 (timer) ve IRQ1 (keyboard) aktif.\n");

    while (1) {
        asm volatile("hlt");
    }
}
