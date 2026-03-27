#ifndef KPRINTF_H
#define KPRINTF_H

#include <stdarg.h>

void kprintf(const char *fmt, ...);
void kvprintf(const char *fmt, va_list args);

#endif
