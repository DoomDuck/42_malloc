#pragma once

#define LOG_FD 2

typedef enum e_log_level {
    log_level_trace,
    log_level_debug,
    log_level_info,
    log_level_warn,
    log_level_error,
    log_level_off,
} log_level;

const char* log_level_name(log_level self);
log_level log_level_from_name(const char* name);

void log_at_level(log_level level, const char* fmt, ...);

// Short hands for the above function
#define log_trace(...) (log_at_level(log_level_trace, __VA_ARGS__))
#define log_debug(...) (log_at_level(log_level_debug, __VA_ARGS__))
#define log_info(...) (log_at_level(log_level_info, __VA_ARGS__))
#define log_warn(...) (log_at_level(log_level_warn, __VA_ARGS__))
#define log_error(...) (log_at_level(log_level_error, __VA_ARGS__))

_Noreturn void assertion_fail(
    const char* assertion,
    const char* file_name,
    unsigned line,
    const char* function_name,
    const char* format,
    ...
);

_Noreturn void fatal(const char* reason);

/* Assert for certain properties in the program's execution */
#define assert(condition, ...) \
    if (!(condition)) \
        assertion_fail(#condition, __FILE__, __LINE__, __func__, __VA_ARGS__);
