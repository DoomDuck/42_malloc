#include <mallok/allocator.h>
#include <mallok/area_list.h>
#include <mallok/log.h>
#include <mallok/print.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include "unistd.h"

#ifndef NO_STYLING
# define RED "\033[38;5;1m"
# define ORANGE "\033[38;5;208m"
# define YELLOW "\033[38;5;3m"
# define GREEN "\033[38;5;10m"
# define BLUE "\033[38;5;12m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"
#define RESET "\033[0m"
#else
# define RED ""
# define ORANGE ""
# define YELLOW ""
# define GREEN ""
# define BLUE ""
# define BOLD ""
# define UNDERLINE ""
# define RESET ""
#endif


bool string_equal(const char* a, const char* b) {
    size_t i = 0;
    for (; a[i] && a[i] == b[i]; ++i)
        ;
    return a[i] == b[i];
}

const char* log_level_name(log_level self) {
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

const char* log_level_color(log_level self) {
    switch (self) {
        case log_level_trace:
            return BLUE;
        case log_level_debug:
            return GREEN;
        case log_level_info:
            return YELLOW;
        case log_level_warn:
            return ORANGE;
        case log_level_error:
            return RED;
		default:
            return "";
    }
}

log_level log_level_from_name(const char* name) {
    for (log_level level = log_level_trace; level <= log_level_off; ++level) {
        if (string_equal(name, log_level_name(level)))
            return level;
    }
    return log_level_off;
}

void log_at_level(log_level level, const char* fmt, ...) {
    if (level < global_allocator.logging_level) return;

	print_fmt(
		LOG_FD, "[%s%s" RESET "] : ",
		log_level_color(level),
		log_level_name(level)
	);

    va_list arg_list;
    va_start(arg_list, fmt);
	print_fmtv(LOG_FD, fmt, arg_list);
    va_end(arg_list);

    print_string(LOG_FD, "\n");
}

_Noreturn void fatal(const char* reason) {
	print_fmt(STDERR_FILENO, RED ">> " "FATAL: %s\n" RESET, reason);
	exit(1);
}

_Noreturn void assertion_fail(
	const char* assertion,
	const char* file_name,
	unsigned line,
	const char* function_name,
	const char* format,
	...
) {
	print_fmt(
		STDERR_FILENO,
		"\n" RED BOLD ">> Failed assertion" RESET
		RED " : "
		"[%s] at " UNDERLINE "%s:%z" 
		RESET RED " in `%s`:\n\t"
		RESET,
		assertion, file_name, line,
		function_name
	);
	va_list args;
	va_start(args, format);
	print_fmtv(STDERR_FILENO, format, args);
	va_end(args);
	print_string(STDERR_FILENO, "\n\n");
	exit(1);
}
