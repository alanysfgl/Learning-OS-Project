// logger.h
#ifndef LOGGER_H
#define LOGGER_H

enum log_level {
    LOG_INFO = 0,
    LOG_WARN = 1,
    LOG_ERR  = 2
};

void log_set_level(enum log_level level);
void log_message(enum log_level level, const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_err(const char *fmt, ...);

#endif
