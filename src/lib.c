#include "mem.h"
#include <internals.h>
#include <log.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/mman.h>

void show_alloc_mem(void) {
	// TODO: use printing_instead of logging functions
	log_info("tiny:\n%P", &global_allocator.tiny);
	log_info("small:\n%P", &global_allocator.small);
	log_info("large:\n%P", &global_allocator.large);
}

void *malloc(size_t allocation_size) {
	log_trace("%z <- malloc", allocation_size);

	void *result = allocator_alloc(&global_allocator, allocation_size);

	return result;
}

void free(void *ptr) {
	log_trace("%p <- free", ptr);

	// Noop on null pointer
	if (!ptr) {
		// TODO: remove
		show_alloc_mem();
		return;
	}

	chunk *c = chunk_of_payload(ptr);
	page *p = page_of_chunk(c);

	chunk *next = chunk_next(c);
	if (next) {
		// next->previous_chunk_size
	}

	// Delete page
	page_deinit(p);
}

void *realloc(void *ptr, size_t new_size) {
	log_trace("ptr = %p, new_size = %z <- realloc", ptr, new_size);

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
