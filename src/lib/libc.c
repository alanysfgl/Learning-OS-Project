// libc.c
#include "libc.h"

void *memset(void *dest, int value, u32int count) {
    u8int *d = (u8int *)dest;
    u8int v = (u8int)value;
    for (u32int i = 0; i < count; i++) {
        d[i] = v;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, u32int count) {
    u8int *d = (u8int *)dest;
    const u8int *s = (const u8int *)src;
    for (u32int i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}

int memcmp(const void *a, const void *b, u32int count) {
    const u8int *x = (const u8int *)a;
    const u8int *y = (const u8int *)b;
    for (u32int i = 0; i < count; i++) {
        if (x[i] != y[i]) {
            return (int)x[i] - (int)y[i];
        }
    }
    return 0;
}

u32int strlen(const char *str) {
    u32int len = 0;
    while (str && str[len]) {
        len++;
    }
    return len;
}

int strcmp(const char *a, const char *b) {
    if (a == b) return 0;
    if (!a) return -1;
    if (!b) return 1;
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return (int)(*(const unsigned char *)a) - (int)(*(const unsigned char *)b);
}

int strncmp(const char *a, const char *b, u32int n) {
    if (n == 0) return 0;
    if (a == b) return 0;
    if (!a) return -1;
    if (!b) return 1;
    while (n-- && *a && (*a == *b)) {
        if (n == 0) return 0;
        a++;
        b++;
    }
    return (int)(*(const unsigned char *)a) - (int)(*(const unsigned char *)b);
}
