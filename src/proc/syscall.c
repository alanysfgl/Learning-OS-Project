// syscall.c
#include "syscall.h"
#include "kprintf.h"
#include "monitor.h"
#include "keyboard.h"
#include "scheduler.h"
#include "ipc.h"

static int user_ptr_ok(const void *ptr, u32int size) {
    if (!ptr || size == 0) return 0;
    u32int addr = (u32int)ptr;
    u32int end = addr + size;
    if (end < addr) return 0;
    if (addr >= 0xC0000000 || end >= 0xC0000000) return 0;
    return 1;
}

static int sys_read(u32int fd, u8int *buf, u32int size) {
    if (fd != 0 || !user_ptr_ok(buf, size)) return -1;
    u32int read = 0;
    while (read < size) {
        int c;
        while ((c = keyboard_getchar()) < 0) {
            asm volatile("sti; hlt");
        }
        buf[read++] = (u8int)c;
        if (c == '\n') break;
    }
    return (int)read;
}

static int sys_write(u32int fd, const u8int *buf, u32int size) {
    if (fd != 1 && fd != 2) return -1;
    if (!user_ptr_ok(buf, size)) return -1;
    for (u32int i = 0; i < size; i++) {
        monitor_put((char)buf[i]);
    }
    return (int)size;
}

static int sys_open(const char *path) {
    if (!user_ptr_ok(path, 1)) return -1;
    return -1;
}

static void sys_exit(int code) {
    kprintf("User exit: %d\n", code);
    scheduler_exit(code);
}

void syscall_handler(struct registers *regs) {
    u32int num = regs->eax;
    switch (num) {
        case SYSCALL_READ:
            regs->eax = (u32int)sys_read(regs->ebx, (u8int *)regs->ecx, regs->edx);
            break;
        case SYSCALL_WRITE:
            regs->eax = (u32int)sys_write(regs->ebx, (const u8int *)regs->ecx, regs->edx);
            break;
        case SYSCALL_OPEN:
            regs->eax = (u32int)sys_open((const char *)regs->ebx);
            break;
        case SYSCALL_EXIT:
            sys_exit((int)regs->ebx);
            regs->eax = 0;
            break;
        case SYSCALL_IPC_SEND:
            regs->eax = (u32int)ipc_send(regs->ebx);
            break;
        case SYSCALL_IPC_RECV: {
            if (!user_ptr_ok((void *)regs->ebx, sizeof(u32int))) {
                regs->eax = (u32int)-1;
                break;
            }
            u32int msg = 0;
            int r = ipc_recv(&msg);
            if (r == 0) {
                *(u32int *)regs->ebx = msg;
            }
            regs->eax = (u32int)r;
            break;
        }
        default:
            kprintf("Unknown syscall: %u\n", num);
            regs->eax = (u32int)-1;
            break;
    }
}
