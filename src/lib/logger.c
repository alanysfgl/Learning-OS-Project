// logger.c
#include "logger.h"
#include "kprintf.h"
#include <stdarg.h>

static enum log_level current_level = LOG_INFO;

static void vlog_message(enum log_level level, const char *fmt, va_list args) {
    static const char *level_names[] = {"INFO", "WARN", "ERR"};

    if (level < current_level) {
        return;
    }

    kprintf("[%s] ", level_names[level]);
    kvprintf(fmt, args);
}

void log_set_level(enum log_level level) {
    current_level = level;
}

void log_message(enum log_level level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlog_message(level, fmt, args);
    va_end(args);
}

void log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlog_message(LOG_INFO, fmt, args);
    va_end(args);
}

void log_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlog_message(LOG_WARN, fmt, args);
    va_end(args);
}

void log_err(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlog_message(LOG_ERR, fmt, args);
    va_end(args);
}
