#pragma once

#include <stddef.h>

#define LOG_FD 2

void log_str(const char *s);
void log_line(const char *s);

void log_size_t(size_t n);
void log_pointer(void *p);

void log_fmt(const char *fmt, ...);
