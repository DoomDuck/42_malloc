#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ALLOC_COUNT 1000

void print(const char *s) { write(1, s, strlen(s)); }

void assert(bool condition, const char *message) {
	if (condition)
		return;
	print(message);
	exit(1);
}

size_t next_alloc_size(void) {
	int category = rand() % 3;
	if (category == 0)
		return (size_t)rand() % 128;
	else if (category == 1)
		return (size_t)rand() % 1024;
	return (size_t)rand() % 10240;
}

void unoptimized_free(void *ptr) { free(ptr); }

int main(void) {
	char *result;
	char *allocations[ALLOC_COUNT];

	srand(0);
	for (size_t i = 0; i < ALLOC_COUNT; ++i) {
		size_t size = next_alloc_size();

		result = (char *)malloc(size);
		assert(size == 0 || result != NULL, "Memory allocation failed");

		for (size_t j = 0; j < size; j++)
			result[j] = (char)rand();

		allocations[i] = result;
	}
	print("Check\n");
	srand(0);
	for (size_t i = 0; i < ALLOC_COUNT; ++i) {
		size_t size = next_alloc_size();
		for (size_t j = 0; j < size; ++j)
			assert(allocations[i][j] == (char)rand(), "Corrupted\n");
	}
	print("OK\n");
	unoptimized_free(NULL);
}
