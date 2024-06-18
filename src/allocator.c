#include <log.h>
#include <mem.h>
#include <page.h>
#include <chunk.h>
#include <utils.h>
#include <page_list.h>
#include <page_type.h>
#include <allocator.h>

#include <stdalign.h>
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

void *allocator_alloc(allocator *self, size_t allocation_size) {
	log_trace("self = %p, allocation_size = %z <- allocator_alloc", self,
	          allocation_size);

	if (allocation_size < sizeof(chunk))
		allocation_size = sizeof(chunk);
	allocation_size = round_up_to_multiple(allocation_size, alignof(chunk));
	
	page_type type = page_type_for_allocation_size(allocation_size);
	page_list *list = allocator_page_list(self, type);
	page* p = NULL;
	chunk * c = NULL; 

	if (type != page_type_large)
		page_list_available_chunk(list, allocation_size, &p);

	if (!c) {
		size_t page_size = page_type_size(type, allocation_size);
		log_trace("No available chunk, creating one of size %z", page_size);
		if (!(p = page_list_insert(list, page_size)))
			return NULL;
		c = p->free;
	}

	page_try_split(p, c, allocation_size);

	page_mark_in_use(p, c);

	return &c->body.payload;
}

void allocator_dealloc(allocator* self, void* address) {
	chunk *c = chunk_of_payload(address);
	page *p = page_of_chunk(c);

	page_type type = page_type_for_allocation_size(chunk_body_size(c));
	page_list* list = allocator_page_list(self, type);

	page_mark_free(p, c);

	page_try_fuse(p, c);

	chunk* previous = chunk_previous(c);
	if (previous)
		page_try_fuse(p, previous);

	if (page_is_empty(p)) page_list_remove(list, p);
}

void* allocator_realloc(allocator *self, void* address, size_t new_size) {
	// On null pointer realloc is equivalent to malloc
	if (!address)
		return allocator_alloc(self, new_size);

	chunk *c = chunk_of_payload(address);

	size_t previous_size = chunk_size(c);

	void *new_place = allocator_alloc(self, new_size);

	size_t copied_amount = previous_size;
	if (previous_size > new_size)
		copied_amount = new_size;
	memory_copy(new_place, address, copied_amount);

	allocator_dealloc(self, address);

	return new_place;

}
