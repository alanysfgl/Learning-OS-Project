// userlib.c
#include "userlib.h"

int sys_read(int fd, void *buf, u32int size) {
    int ret;
    asm volatile("int $0x80"
                 : "=a"(ret)
                 : "a"(0), "b"(fd), "c"(buf), "d"(size)
                 : "memory");
    return ret;
}

int sys_write(int fd, const void *buf, u32int size) {
    int ret;
    asm volatile("int $0x80"
                 : "=a"(ret)
                 : "a"(1), "b"(fd), "c"(buf), "d"(size)
                 : "memory");
    return ret;
}

int sys_open(const char *path) {
    int ret;
    asm volatile("int $0x80"
                 : "=a"(ret)
                 : "a"(2), "b"(path)
                 : "memory");
    return ret;
}

void sys_exit(int code) {
    asm volatile("int $0x80"
                 :
                 : "a"(3), "b"(code)
                 : "memory");
    for (;;) { }
}

int sys_ipc_send(u32int msg) {
    int ret;
    asm volatile("int $0x80"
                 : "=a"(ret)
                 : "a"(4), "b"(msg)
                 : "memory");
    return ret;
}

int sys_ipc_recv(u32int *msg) {
    int ret;
    asm volatile("int $0x80"
                 : "=a"(ret)
                 : "a"(5), "b"(msg)
                 : "memory");
    return ret;
}
