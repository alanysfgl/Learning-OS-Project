// common.h
#ifndef COMMON_H
#define COMMON_H

typedef unsigned int   u32int;
typedef          int   s32int;
typedef unsigned short u16int;
typedef          short s16int;
typedef unsigned char  u8int;
typedef          char  s8int;
typedef unsigned long long u64int;
typedef          long long s64int;

static inline void outb(u16int port, u8int value) {
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

static inline u8int inb(u16int port) {
    u8int ret;
    asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

static inline void outw(u16int port, u16int value) {
    asm volatile ("outw %1, %0" : : "dN" (port), "a" (value));
}

static inline u16int inw(u16int port) {
    u16int ret;
    asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

static inline void outl(u16int port, u32int value) {
    asm volatile ("outl %1, %0" : : "dN" (port), "a" (value));
}

static inline u32int inl(u16int port) {
    u32int ret;
    asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

static inline void cpuid(u32int code, u32int *a, u32int *b, u32int *c, u32int *d) {
    u32int eax, ebx, ecx, edx;
    asm volatile ("cpuid"
                  : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                  : "a"(code));
    if (a) *a = eax;
    if (b) *b = ebx;
    if (c) *c = ecx;
    if (d) *d = edx;
}

#endif
