#include <log.h>
#include <print.h>

void log_str(const char* s) {
	print_string(LOG_FD, s);
}

void log_line(const char* s) {
	print_string(LOG_FD, s);
	print_string(LOG_FD, "\n");
}

void log_size_t(size_t n) {
	print_size_t(LOG_FD, n);
}

void log_pointer(void* p) {
	print_pointer(LOG_FD, p);
}
