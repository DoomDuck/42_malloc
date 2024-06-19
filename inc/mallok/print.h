#pragma once

#include <mallok/area.h>
#include <mallok/area_list.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>

/* File descriptor */
typedef int fd;

bool write_all(fd output, const void* bytes, size_t count);

size_t string_length(const char* s);

void print_string(fd output, const char* s);

void print_size_t(fd output, size_t n);

void print_pointer(fd output, void* p);

void print_area(fd output, area* a);

void print_area_list(fd output, area_list* a);

void print_fmt(fd output, const char* fmt, ...);

void print_fmtv(fd output, const char* fmt, va_list arg_list);
