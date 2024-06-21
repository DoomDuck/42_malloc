#include <errno.h>
#include <mallok/area_list.h>
#include <mallok/print.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
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

static inline char hex_digit(uint8_t index) {
    if (index < 10)
        return '0' + index;
    return 'a' + (index - 10);
}

void print_pointer(fd output, void* p) {
    char buffer[26];
    char* cursor = &buffer[sizeof(buffer)];

    uintptr_t n = (uintptr_t)p;

    *--cursor = '\0';
    do {
        *--cursor = hex_digit(n % 16);
        n /= 16;
    } while (n > 0);

    // Add prefix
    *--cursor = 'x';
    *--cursor = '0';

    print_string(output, cursor);
}

void print_chunk_dump(fd output, chunk* c) {
    if (!c->header.in_use)
        return;

    uint8_t* cursor = &c->body.payload;
    uint8_t* end = cursor + chunk_body_size(c);

    char line_buffer[128];
    char* write_head = line_buffer;
    size_t bytes_displayed_this_line = 0;
    while (cursor < end) {
        // Write prefix
        if (bytes_displayed_this_line == 0) {
            write_head += 16;
            char* reversed_write_head = write_head;
            uintptr_t p = (uintptr_t)cursor;
            for (size_t i = 0; i < sizeof(uintptr_t) * 2; ++i) {
                *--reversed_write_head = hex_digit(p & 0xf);
                p >>= 4;
            }

            // Write suffix
            *write_head++ = ' ';
            *write_head++ = '|';
            *write_head++ = ' ';
        }

        // Add hex pair to line
        uint8_t value = *cursor++;
        *write_head++ = hex_digit(value >> 4);
        *write_head++ = hex_digit(value & 0xf);

        if (++bytes_displayed_this_line >= 16) {
            // End of line
            *write_head++ = '\n';
            write_all(output, line_buffer, write_head - line_buffer);
            bytes_displayed_this_line = 0;
            write_head = line_buffer;
        } else {
            // New line
            *write_head++ = ' ';
        }
    }
    *write_head++ = '\n';
    write_all(output, line_buffer, write_head - line_buffer);
}

void print_area(fd output, area* a, bool dump_hex) {
    chunk* cursor = &a->first_chunk;

    for (; cursor; cursor = chunk_next(cursor)) {
        if (!cursor->header.in_use)
            continue;

        size_t alloc_size = chunk_body_size(cursor);
        print_fmt(
            output,
            "\t %p - %p - : %z bytes\n",
            &cursor->body.payload,
            &cursor->body.payload + alloc_size,
            alloc_size
        );

        if (dump_hex)
            print_chunk_dump(output, cursor);
    }
}

void print_area_list(fd output, area_list* list, bool dump_hex) {
    area_list_node* cursor = list->first;

    print_fmt(output, "\t first = %p\n", list->first);

    for (; cursor; cursor = cursor->next) {
        print_area(output, &cursor->area, dump_hex);
    }
}

void print_fmt(fd output, const char* fmt, ...) {
    va_list arg_list;
    va_start(arg_list, fmt);
    print_fmtv(output, fmt, arg_list);
    va_end(arg_list);
}

void print_fmtv(fd output, const char* fmt, va_list arg_list) {
    size_t i = 0;
    size_t written_up_to = 0;
    bool in_format = false;
    for (i = 0; fmt[i]; ++i) {
        char c = fmt[i];
        if (in_format) {
            in_format = false;
            if (c == 'p') {
                print_pointer(output, va_arg(arg_list, void*));
            } else if (c == 's') {
                print_string(output, va_arg(arg_list, char*));
            } else if (c == 'z') {
                print_size_t(output, va_arg(arg_list, size_t));
            } else if (c == 'a' || c == 'A') {
                print_area(output, va_arg(arg_list, area*), c == 'A');
            } else if (c == 'l' || c == 'L') {
                print_area_list(output, va_arg(arg_list, area_list*), c == 'L');
            } else if (c == 'e') {
                print_string(output, strerror(errno));
            } else {
                continue;
            }
            written_up_to = i + 1;
        } else if (fmt[i] == '%') {
            in_format = true;
            write_all(output, &fmt[written_up_to], i - written_up_to);
            written_up_to = i;
            continue;
        }
    }
    write_all(output, &fmt[written_up_to], i - written_up_to);
}
