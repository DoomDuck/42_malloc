#include <malloc.h>
#include <internals.h>

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

void chunk_init_alloc(chunk* self, size_t previous_chunk_size, size_t size) {
	self->previous_chunk_size = previous_chunk_size;
	// TODO: assert that: size >= sizeof(chunk)
	// TODO: check size is a multiple of 16
	self->header.size_divided_by_16 = size / 16;
}

size_t chunk_size(chunk* self) {
	return self->header.size_divided_by_16 * 16;
}

chunk* chunk_of_payload(void *payload) {
	size_t offset = (size_t)(&((chunk*)NULL)->body.payload);
	return (chunk*)((uint8_t*)payload - offset);
}
