#pragma once

#include <stdbool.h>
#include <unistd.h>

/* File descriptor */
typedef int fd;

bool write_all(fd output, const void* bytes, size_t count);

size_t string_length(const char* s);

void print_string(fd output, const char* s);

void print_size_t(fd output, size_t n);

void print_pointer(fd output, void* p);
