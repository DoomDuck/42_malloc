#include "internals.h"
#include <log.h>
#include <print.h>
#include <stdarg.h>
#include <stdbool.h>

/* TODO: assess if I can use them */
#include <string.h>
#include <errno.h>

void log_str(const char *s) { print_string(LOG_FD, s); }

void log_line(const char *s) {
	print_string(LOG_FD, s);
	print_string(LOG_FD, "\n");
}

void log_size_t(size_t n) { print_size_t(LOG_FD, n); }

void log_pointer(void *p) { print_pointer(LOG_FD, p); }

void log_fmt(const char *fmt, ...) {
	size_t i = 0;
	va_list arg_list;

	size_t written_up_to = 0;
	bool in_format = false;

	va_start(arg_list, fmt);
	for (i = 0; fmt[i]; ++i) {
		char c = fmt[i];
		if (in_format) {
			if (c == 'p') {
				log_pointer(va_arg(arg_list, void *));
			} else if (c == 's') {
				log_str(va_arg(arg_list, char *));
			} else if (c == 'z') {
				log_size_t(va_arg(arg_list, size_t));
			} else if (c == 'P') {
				page_list_show(va_arg(arg_list, page_list_node*));
			} else if (c == 'e') {
				log_str(strerror(errno));
			} else {
				continue;
			}
			written_up_to = i + 1;
		} else if (fmt[i] == '%') {
			in_format = true;
			write_all(LOG_FD, &fmt[written_up_to], i - written_up_to);
			written_up_to = i;
			continue;
		}
		in_format = false;
	}
	va_end(arg_list);
	write_all(LOG_FD, &fmt[written_up_to], i - written_up_to);
}
