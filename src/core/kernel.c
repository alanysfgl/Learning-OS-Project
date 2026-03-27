#include "idt.h"
#include "gdt.h"
#include "monitor.h"
#include "keyboard.h"
#include "timer.h"
#include "kprintf.h"
#include "multiboot.h"
#include "logger.h"
#include "memory.h"
#include "paging.h"
#include "heap.h"
#include "scheduler.h"
#include "mouse.h"
#include "serial.h"
#include "ata.h"
#include "vfs.h"
#include "ramfs.h"
#include "fat.h"
#include "shell.h"
#include "input.h"
#include "init.h"
#include "ipc.h"
#include "rtl8139.h"
#include "smp.h"

void kernel_main(u32int magic, u32int addr) {
    init_gdt();
    init_idt();
    serial_init();
    monitor_enable_serial(1);
    log_set_level(LOG_INFO);
    input_init();
    ipc_init();
    vfs_init();
    ramfs_init();
    fat_init();
    struct multiboot_info *mb = multiboot_init(magic, addr);
    frame_allocator_init(mb);
    paging_init();
    heap_init(0xC1000000, 1024 * 1024);
    set_kernel_stack(0xC1100000);
    scheduler_init();
    timer_init(100);
    keyboard_init();
    serial_enable_rx_interrupts();
    mouse_init();
    ata_init();
    rtl8139_init();
    smp_init();
    monitor_write("Kernel basladi. Timer ve klavye aktif.\n");
    scheduler_add(init_task);
    scheduler_start();
    // When user-mode tasks exit, we return here; drop into kernel shell.
    shell_run();
    while(1) { asm volatile("hlt"); }
}
