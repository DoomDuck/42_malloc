#include "mem.h"
#include <internals.h>
#include <log.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

void show_alloc_mem(void) {
	log_fmt("tiny: %P\n", &global_allocator.tiny);
	log_fmt("small: %P\n", &global_allocator.small);
	log_fmt("large: %P\n", &global_allocator.large);
}

void *malloc(size_t allocation_size) {
	log_fmt("%z <- malloc\n", allocation_size);

	// For now only use large page algorithm
	// TODO: Lookup good page type
	page_type type = page_type_for_allocation_size(allocation_size);
	// page_type type = page_type_large;

	page *p = page_new(type, allocation_size);

	if (!p)
		return NULL;

	log_fmt("%p <- malloc return\n", &p->first_chunk.body.payload);
	return &p->first_chunk.body.payload;
}

void free(void *ptr) {
	log_fmt("%p <- free\n", ptr);

	// Noop on null pointer
	if (!ptr)
		return;

	chunk *c = chunk_of_payload(ptr);
	// TODO: change later on
	page *p = page_of_first_chunk(c);

	// Delete page
	page_deinit(p);
}

void *realloc(void *ptr, size_t new_size) {
	log_fmt("ptr = %p, new_size = %z <- realloc\n", ptr, new_size);

	// On null ptr realloc is equivalent to malloc
	if (!ptr)
		return malloc(new_size);

	chunk *c = chunk_of_payload(ptr);

	size_t previous_size = chunk_size(c);

	void *new_place = malloc(new_size);

	size_t copied_amount = previous_size;
	if (previous_size > new_size)
		copied_amount = new_size;
	memory_copy(new_place, ptr, copied_amount);

	free(ptr);

	return new_place;
}
