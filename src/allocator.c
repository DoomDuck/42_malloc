#include "malloc.h"
#include <internals.h>
#include <log.h>
#include <stdalign.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

allocator global_allocator;

void allocator_init(allocator *self) {
	self->page_size = getpagesize();
	log_info("%z <- page size", self->page_size);
	// TODO: initalize minimal amount of pages
	page_list_init(&self->tiny);
	page_list_init(&self->small);
	page_list_init(&self->large);
}

page_list *allocator_page_list(allocator *self, page_type type) {
	switch (type) {
	default:
	case page_type_tiny:
		return &self->tiny;
	case page_type_small:
		return &self->small;
	case page_type_large:
		return &self->large;
	}
}

void global_allocator_init(void) { allocator_init(&global_allocator); }

chunk_list_ref *allocator_available_chunk(allocator *self,
                                          size_t allocation_size) {
	switch (page_type_for_allocation_size(allocation_size)) {
	case page_type_tiny:
		return page_list_available_chunk(&self->tiny, allocation_size);
	case page_type_small:
		return page_list_available_chunk(&self->small, allocation_size);
	default:
		return NULL;
	}
}

void *allocator_alloc(allocator *self, size_t allocation_size) {
	log_trace("self = %p, allocation_size = %z <- allocator_alloc", self,
	          allocation_size);

	if (allocation_size < sizeof(chunk))
		allocation_size = sizeof(chunk);
	allocation_size = round_up_to_multiple(allocation_size, alignof(chunk));

	chunk_list_ref *ref = allocator_available_chunk(self, allocation_size);
	if (!ref) {
		log_trace("No available chunk, creating one");
		page_type type = page_type_for_allocation_size(allocation_size);
		size_t page_size = page_type_size(type, allocation_size);
		page_list *list = allocator_page_list(self, type);
		page *page_ptr = page_list_insert(list, page_size);
		if (!page_ptr)
			return NULL;
		ref = &page_ptr->first_free;
	}

	chunk_try_split(*ref, allocation_size);

	chunk *c = chunk_extract_from_list(ref);

	chunk *next = chunk_next(c);
	if (next)
		next->header.previous_in_use = true;

	return &c->body.payload;
}
