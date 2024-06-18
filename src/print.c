#include <mallok/print.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

size_t string_length(const char* s) {
    size_t length = 0;
    while (s[length])
        length++;
    return length;
}

bool write_all(fd output, const void* bytes, size_t count) {
    while (count != 0) {
        ssize_t result = write(output, bytes, count);
        if (result < 0)
            return false;
        count -= result;
        bytes = (uint8_t*)bytes + result;
    }
    return true;
}

void print_string(fd output, const char* s) {
    write_all(output, s, string_length(s));
}

void print_size_t(fd output, size_t n) {
    char buffer[26];
    char* cursor = &buffer[sizeof(buffer)];

    *--cursor = '\0';
    do {
        *--cursor = '0' + (n % 10);
        n /= 10;
    } while (n > 0);

    print_string(output, cursor);
}

void print_pointer(fd output, void* p) {
    char buffer[26];
    char* cursor = &buffer[sizeof(buffer)];

    size_t n = (size_t)p;

    *--cursor = '\0';
    do {
        size_t rest = (n % 16);
        if (rest < 10)
            *--cursor = '0' + rest;
        else
            *--cursor = 'a' + (rest - 10);
        n /= 16;
    } while (n > 0);

    // Add prefix
    *--cursor = 'x';
    *--cursor = '0';

    print_string(output, cursor);
}
