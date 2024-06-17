#include <log.h>
#include <page_list.h>
#include <print.h>

#include <stdarg.h>
#include <stdbool.h>

/* TODO: assess if I can use them */
#include <errno.h>
#include <string.h>

const char *log_level_name(log_level self) {
	switch (self) {
	case log_level_trace:
		return "trace";
	case log_level_debug:
		return "debug";
	case log_level_info:
		return "info";
	case log_level_warn:
		return "warn";
	case log_level_error:
		return "error";
	case log_level_off:
		return "off";
	}
	return "unknown";
}

void log_at_level(log_level level, const char *fmt, ...) {
	if (level < LOG_LEVEL)
		return;

	print_string(LOG_FD, "[");
	print_string(LOG_FD, log_level_name(level));
	print_string(LOG_FD, "] : ");

	size_t i = 0;
	va_list arg_list;

	size_t written_up_to = 0;
	bool in_format = false;

	va_start(arg_list, fmt);
	for (i = 0; fmt[i]; ++i) {
		char c = fmt[i];
		if (in_format) {
			in_format = false;
			if (c == 'p') {
				print_pointer(LOG_FD, va_arg(arg_list, void *));
			} else if (c == 's') {
				print_string(LOG_FD, va_arg(arg_list, char *));
			} else if (c == 'z') {
				print_size_t(LOG_FD, va_arg(arg_list, size_t));
			} else if (c == 'P') {
				page_list_show(va_arg(arg_list, page_list *));
			} else if (c == 'e') {
				print_string(LOG_FD, strerror(errno));
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
	}
	va_end(arg_list);
	write_all(LOG_FD, &fmt[written_up_to], i - written_up_to);
	print_string(LOG_FD, "\n");
}
