#include <mallok/area_list.h>
#include <mallok/log.h>
#include <mallok/print.h>
#include <stdarg.h>
#include <stdbool.h>

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

log_level log_level_from_name(const char* name) {
    for (log_level level = log_level_trace; level <= log_level_off; ++level) {
        if (string_equal(name, log_level_name(level)))
            return level;
    }
    return log_level_off;
}

void log_at_level(log_level level, const char* fmt, ...) {
    if (level < LOG_LEVEL) return;

	print_fmt(LOG_FD, "[%s]", log_level_name(level));

    va_list arg_list;
    va_start(arg_list, fmt);
	print_vfmt(LOG_FD, fmt, arg_list);
    va_end(arg_list);

    print_string(LOG_FD, "\n");
}
