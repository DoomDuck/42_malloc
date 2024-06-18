#include <mallok/allocator.h>
#include <mallok/chunk.h>
#include <mallok/log.h>
#include <mallok/mem.h>
#include <mallok/page.h>
#include <mallok/page_list.h>
#include <mallok/utils.h>

#include <stdalign.h>
#include <stdlib.h>
#include <unistd.h>

allocator global_allocator;

void allocator_init(allocator *self) {
	const char *log_env_var = getenv("MALLOK_LOG");
	self->logging_level = log_level_error;
	if (log_env_var)
		self->logging_level = log_level_from_name(log_env_var);
	self->page_size = getpagesize();
	log_info("%z <- page size", self->page_size);
	// TODO: initalize minimal amount of pages
	page_list_init(&self->tiny);
	page_list_init(&self->small);
	page_list_init(&self->large);
}

size_t allocator_page_size_for_size(allocator* self, size_t allocation_size) {
	if (allocation_size <= PAGE_TINY_MAX_ALLOCATION_SIZE)
		return self->page_size;
	else if (allocation_size <= PAGE_SMALL_MAX_ALLOCATION_SIZE)
		return 3 * self->page_size;
	else
		return round_up_to_multiple(
			allocation_size + PAGE_HEADER_SIZE + CHUNK_HEADER_SIZE,
		    global_allocator.page_size
		);
}

page_list* allocator_page_list_for_size(allocator* self, size_t allocation_size) {
	if (allocation_size <= PAGE_TINY_MAX_ALLOCATION_SIZE)
		return &self->tiny;
	else if (allocation_size <= PAGE_SMALL_MAX_ALLOCATION_SIZE)
		return &self->small;
	else
		return &self->large;
}

void global_allocator_init(void) { allocator_init(&global_allocator); }

void *allocator_alloc(allocator *self, size_t allocation_size) {
	log_trace("self = %p, allocation_size = %z <- allocator_alloc", self,
	          allocation_size);

	if (allocation_size < sizeof(chunk))
		allocation_size = sizeof(chunk);
	allocation_size = round_up_to_multiple(allocation_size, alignof(chunk));

	page_list *list = allocator_page_list_for_size(self, allocation_size);
	page *p = NULL;
	chunk *c = NULL;

	if (list != &self->large)
		c = page_list_available_chunk(list, allocation_size, &p);

	if (!c) {
		size_t page_size = allocator_page_size_for_size(self, allocation_size);
		log_trace("No available chunk, creating one of size %z", page_size);
		if (!(p = page_list_insert(list, page_size)))
			return NULL;
		c = p->free;
	}

	page_try_split(p, c, allocation_size);

	page_mark_in_use(p, c);

	return &c->body.payload;
}

void allocator_dealloc(allocator *self, void *address) {
	chunk *c = chunk_of_payload(address);
	page *p = page_of_chunk(c);

	page_list *list = allocator_page_list_for_size(self, chunk_body_size(c));

	page_mark_free(p, c);

	page_try_fuse(p, c);

	chunk *previous = chunk_previous(c);
	if (!c->header.previous_in_use && previous)
		page_try_fuse(p, previous);

	if (page_is_empty(p))
		page_list_remove(list, p);
}

void *allocator_realloc(allocator *self, void *address, size_t new_size) {
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
