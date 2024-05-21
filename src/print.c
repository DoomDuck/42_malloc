#include <print.h>
#include <stddef.h>
#include <unistd.h>

size_t string_length(const char* s) {
	size_t length = 0;
	while (s[length])
		length++;
	return length;
}

void print_string(fd output, const char* s) {
	write(output, s, string_length(s));
}

void print_size_t(fd output, size_t n) {
	char buffer[20];
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
		if (rest < 10) *--cursor = '0' + rest;
		else *--cursor = 'a' + (rest - 10);
		n /= 16;
	} while (n > 0);
	
	// Add prefix
	*--cursor = 'x';
	*--cursor = '0';

	print_string(output, cursor);
}
