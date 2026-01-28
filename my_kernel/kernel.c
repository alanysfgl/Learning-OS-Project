#include "idt.h"
#include "gdt.h"

void kernel_main() {
    init_gdt();
    init_idt(); 
    
    while(1) {
        asm volatile("hlt");
    }
}