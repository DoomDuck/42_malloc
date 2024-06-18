#include <mallok/allocator.h>
#include <mallok/chunk.h>
#include <mallok/log.h>
#include <mallok/mem.h>
#include <mallok/area.h>
#include <mallok/area_list.h>
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
	// TODO: initalize minimal amount of areas
	area_list_init(&self->tiny);
	area_list_init(&self->small);
	area_list_init(&self->large);
}

size_t allocator_area_size_for_size(allocator* self, size_t allocation_size) {
	if (allocation_size <= AREA_TINY_MAX_ALLOCATION_SIZE)
		return self->page_size;
	else if (allocation_size <= AREA_SMALL_MAX_ALLOCATION_SIZE)
		return 3 * self->page_size;
	else
		return round_up_to_multiple(
			allocation_size + AREA_HEADER_SIZE + CHUNK_HEADER_SIZE,
		    global_allocator.page_size
		);
}

area_list* allocator_area_list_for_size(allocator* self, size_t allocation_size) {
	if (allocation_size <= AREA_TINY_MAX_ALLOCATION_SIZE)
		return &self->tiny;
	else if (allocation_size <= AREA_SMALL_MAX_ALLOCATION_SIZE)
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

	area_list *list = allocator_area_list_for_size(self, allocation_size);
	area *a = NULL;
	chunk *c = NULL;

	if (list != &self->large)
		c = area_list_available_chunk(list, allocation_size, &a);

	if (!c) {
		size_t area_size = allocator_area_size_for_size(self, allocation_size);
		log_trace("No available chunk, creating one of size %z", area_size);
		if (!(a = area_list_insert(list, area_size)))
			return NULL;
		c = a->free;
	}

	area_try_split(a, c, allocation_size);

	area_mark_in_use(a, c);

	return &c->body.payload;
}

void allocator_dealloc(allocator *self, void *address) {
	chunk *c = chunk_of_payload(address);
	area *a = area_of_chunk(c);

	area_list *list = allocator_area_list_for_size(self, chunk_body_size(c));

	area_mark_free(a, c);

	area_try_fuse(a, c);

	chunk *previous = chunk_previous(c);
	if (!c->header.previous_in_use && previous)
		area_try_fuse(a, previous);

	if (area_is_empty(a))
		area_list_remove(list, a);
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
