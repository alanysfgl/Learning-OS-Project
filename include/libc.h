// libc.h
#ifndef LIBC_H
#define LIBC_H

#include "common.h"

void *memset(void *dest, int value, u32int count);
void *memcpy(void *dest, const void *src, u32int count);
int memcmp(const void *a, const void *b, u32int count);
u32int strlen(const char *str);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, u32int n);

#endif
