// syscall.h
#ifndef SYSCALL_H
#define SYSCALL_H

#include "idt.h"

enum {
    SYSCALL_READ  = 0,
    SYSCALL_WRITE = 1,
    SYSCALL_OPEN  = 2,
    SYSCALL_EXIT  = 3,
    SYSCALL_IPC_SEND = 4,
    SYSCALL_IPC_RECV = 5,
};

void syscall_handler(struct registers *regs);

#endif
