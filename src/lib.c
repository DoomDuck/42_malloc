#include <malloc.h>
#include <log.h>
#include <internals.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

void memory_copy(void* destination, const void* source, size_t count) {
	uint8_t* destination_array = destination;
	const uint8_t* source_array = source;
	for (size_t i = 0; i < count; ++i)
		destination_array[i] = source_array[i];
}

void show_alloc_mem(void) {
	log_str("tiny:\n");
	page_list_show(&global_allocator.tiny);
	log_str("small:\n");
	page_list_show(&global_allocator.small);
	log_str("large:\n");
	page_list_show(&global_allocator.large);
}

void* malloc(size_t allocation_size) {
	log_size_t(allocation_size);
	log_str(" <- malloc\n");

	// For now only use large page algorithm
	// TODO: Lookup good page type
	// 	   page_type type = page_type_for_allocation_size(allocation_size);
	page_type type = page_type_large;

	page* p = page_new(type, allocation_size);

	if (!p) return NULL;

	log_pointer(&p->first_chunk.body.payload);
	log_str(" <- malloc return\n");
	return &p->first_chunk.body.payload;
}

void free(void* ptr) {
	log_pointer(ptr);
	log_str(" <- free\n");

	// Noop on null pointer
	if (!ptr) return;

	chunk* c = chunk_of_payload(ptr);
	// TODO: change later on
	page* p = page_of_first_chunk(c);

	// Delete page
	page_deinit(p);
}

void* realloc(void* ptr, size_t new_size) {
	log_str("ptr = ");
	log_pointer(ptr);
	log_str(", new_size = ");
	log_size_t(new_size);
	log_str(" <- realloc\n");

	// On null ptr realloc is equivalent to malloc
	if (!ptr) return malloc(new_size);

	chunk* c = chunk_of_payload(ptr);

	size_t previous_size = chunk_size(c);

	void* new_place = malloc(new_size);

	size_t copied_amount = previous_size;
	if (previous_size > new_size) copied_amount = new_size;
	memory_copy(new_place, ptr, copied_amount);

	free(ptr);

	return new_place;
}
