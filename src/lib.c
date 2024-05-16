#include <malloc.h>
#include <internals.h>
#include <stdlib.h>
#include <sys/mman.h>

void free(void* ptr) {
	// Noop on null pointer
	if (!ptr) return;

	chunk* c = chunk_of_payload(ptr);
	// TODO: change later on
	page* p = page_of_first_chunk(c);
	munmap(p, p->size);
}

void* malloc(size_t size) {

	// TODO: round size up to next page_size
	page* p = mmap(
		NULL, // To address is required
		size, // Size of the page to request
		PROT_READ | PROT_WRITE, // Allow reading and writing
		MAP_ANONYMOUS // Do not map of filesystem
		| MAP_PRIVATE, // Do not share with other processes
		-1, 0 // unused filesystem specific options
	);

	if (p == MAP_FAILED) {
		// TODO: log
		// Allocation failed
		return NULL;
	}

	return &p->first_chunk.body.payload;
}

void mem_copy(void* destination, const void* source, size_t count) {
	uint8_t* destination_array = destination;
	const uint8_t* source_array = source;
	for (size_t i = 0; i < count; ++i)
		destination_array[i] = source_array[i];
}

void* realloc(void* ptr, size_t new_size) {
	chunk* c = chunk_of_payload(ptr);

	size_t previous_size = chunk_size(c);

	void* new_place = malloc(new_size);
	
	size_t copied_amount = previous_size;
	if (previous_size > new_size) copied_amount = new_size;
	mem_copy(new_place, ptr, copied_amount);

	free(ptr);

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
	// TODO: assert alignment
	return (chunk*)((uint8_t*)payload - offsetof(chunk, body.payload));
}

page* page_of_first_chunk(chunk* first) {
	// TODO: assert alignment
	return (page*)((uint8_t*)first - offsetof(page, first_chunk));
}
