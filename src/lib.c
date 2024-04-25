#include <malloc.h>

void free(void* ptr) {
	(void)ptr;
}

void* malloc(size_t size) {
	(void)size;
	return NULL;
}

void* realloc(void* ptr, size_t size) {
	(void)ptr;
	(void)size;
	return NULL;
}
