#include "kprintf.h"
#include "monitor.h"
#include "common.h"
#include "libc.h"

static void put_padding(int count, char pad) {
    for (int i = 0; i < count; i++) {
        monitor_put(pad);
    }
}

static u64int u64_divmod(u64int n, u32int base, u32int *rem) {
    u64int quotient = 0;
    u64int denom = (u64int)base;
    u64int current = 1;

    if (base == 0) {
        if (rem) *rem = 0;
        return 0;
    }

    while ((denom << 1) > denom && (denom << 1) <= n) {
        denom <<= 1;
        current <<= 1;
    }

    while (current != 0) {
        if (n >= denom) {
            n -= denom;
            quotient |= current;
        }
        denom >>= 1;
        current >>= 1;
    }

    if (rem) *rem = (u32int)n;
    return quotient;
}

static void print_uint_base(u64int value, u32int base, int width, char pad, int uppercase) {
    char buf[32];
    int i = 0;
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

    if (value == 0) {
        buf[i++] = '0';
    } else {
        while (value > 0 && i < (int)sizeof(buf)) {
            u32int digit = 0;
            value = u64_divmod(value, base, &digit);
            buf[i++] = digits[digit];
        }
    }

    if (width > i) {
        put_padding(width - i, pad);
    }
    while (i--) {
        monitor_put(buf[i]);
    }
}

static void print_int_decimal(s64int value, int width, char pad) {
    if (value < 0) {
        monitor_put('-');
        if (width > 0) width--;
        value = -value;
    }
    print_uint_base((u64int)value, 10, width, pad, 0);
}

static void print_string(const char *s, int width, char pad) {
    if (!s) s = "(null)";
    u32int len = strlen(s);
    if (width > (int)len) {
        put_padding(width - (int)len, pad);
    }
    monitor_write((char *)s);
}

void kvprintf(const char *fmt, va_list args) {
    for (u32int i = 0; fmt[i]; i++) {
        if (fmt[i] != '%') {
            monitor_put(fmt[i]);
            continue;
        }
        i++;
        if (!fmt[i]) break;

        char pad = ' ';
        int width = 0;
        if (fmt[i] == '0') {
            pad = '0';
            i++;
        }
        while (fmt[i] >= '0' && fmt[i] <= '9') {
            width = (width * 10) + (fmt[i] - '0');
            i++;
        }

        if (fmt[i] == 'l' && fmt[i + 1] == 'l') {
            i += 2;
            if (!fmt[i]) break;
            if (fmt[i] == 'u') {
                u64int v = va_arg(args, u64int);
                print_uint_base(v, 10, width, pad, 0);
                continue;
            }
            monitor_put('%');
            monitor_put('l');
            monitor_put('l');
            monitor_put(fmt[i]);
            continue;
        }

        switch (fmt[i]) {
            case 'c': {
                char c = (char)va_arg(args, int);
                monitor_put(c);
                break;
            }
            case 's': {
                const char *s = va_arg(args, const char*);
                print_string(s, width, pad);
                break;
            }
            case 'd': {
                int v = va_arg(args, int);
                print_int_decimal((s64int)v, width, pad);
                break;
            }
            case 'u': {
                u32int v = va_arg(args, u32int);
                print_uint_base(v, 10, width, pad, 0);
                break;
            }
            case 'x': {
                u32int v = va_arg(args, u32int);
                print_uint_base(v, 16, width, pad, 1);
                break;
            }
            case 'X': {
                u32int v = va_arg(args, u32int);
                print_uint_base(v, 16, width, pad, 1);
                break;
            }
            case 'p': {
                u32int v = (u32int)va_arg(args, void*);
                int ptr_width = width ? width : 8;
                char ptr_pad = (width == 0) ? '0' : pad;
                monitor_write("0x");
                print_uint_base(v, 16, ptr_width, ptr_pad, 1);
                break;
            }
            case '%':
                monitor_put('%');
                break;
            default:
                monitor_put('%');
                monitor_put(fmt[i]);
                break;
        }
    }
}

void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kvprintf(fmt, args);
    va_end(args);
}
