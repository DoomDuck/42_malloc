#pragma once

#include <stdbool.h>
#include <unistd.h>
#include <stdarg.h>

/* File descriptor */
typedef int fd;

bool write_all(fd output, const void* bytes, size_t count);

size_t string_length(const char* s);

void print_string(fd output, const char* s);

void print_size_t(fd output, size_t n);

void print_pointer(fd output, void* p);

void print_fmt(fd output, const char* fmt, ...);

void print_vfmt(fd output, const char* fmt, va_list arg_list);
