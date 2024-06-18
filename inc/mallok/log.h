#pragma once

/* Required for the assert macro */
#include <stdlib.h>

#define LOG_FD 2

#define LOG_LEVEL (log_level_trace)

typedef enum {
	log_level_trace,
	log_level_debug,
	log_level_info,
	log_level_warn,
	log_level_error,
	log_level_off,
} log_level;

const char *log_level_name(log_level self);

void log_at_level(log_level level, const char *fmt, ...);

// Short hands for the above function
#define log_trace(...) (log_at_level(log_level_trace, __VA_ARGS__))
#define log_debug(...) (log_at_level(log_level_debug, __VA_ARGS__))
#define log_info(...) (log_at_level(log_level_info, __VA_ARGS__))
#define log_warn(...) (log_at_level(log_level_warn, __VA_ARGS__))
#define log_error(...) (log_at_level(log_level_error, __VA_ARGS__))

#define assert(condition, ...)                                                 \
	if (!(condition)) {                                                        \
		log_error(__VA_ARGS__);                                                \
		exit(1);                                                               \
	}
