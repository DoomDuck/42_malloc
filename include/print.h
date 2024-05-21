#pragma once

#include <unistd.h>

/* File descriptor */
typedef int fd;

size_t string_length(const char* s);

void print_string(fd output, const char* s);

void print_size_t(fd output, size_t n);

void print_pointer(fd output, void* p);

