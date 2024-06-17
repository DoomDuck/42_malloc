#include <chunk.h>
#include <log.h>
#include <chunk.h>
#include <page.h>
#include <page_list.h>

void page_init(page *self, size_t size) {
	log_trace("self = %p, size = %z <- page_init", self, size);

	// Initialize struct
	self->size = size;
	self->free = &self->first_chunk;

	// Make first chunk fill all the available space
	chunk_init(&self->first_chunk, 0,
	           size - offsetof(page_list_node, page.first_chunk), false, false,
	           NULL, NULL);
}

void page_deinit(page *self) {
	log_error("This function is not implemented");
	exit(1);
}

bool page_is_empty(page *self) {
	return (!self->first_chunk.header.in_use && !chunk_next(&self->first_chunk));
}

page *page_of_first_chunk(chunk *first) {
	log_trace("page_of_first_chunk");
	page *result = (page *)((uintptr_t)first - offsetof(page, first_chunk));
	return result;
}

page *page_of_chunk(chunk *cursor) {
	while (!chunk_is_first(cursor))
		cursor = chunk_previous(cursor);
	return (page_of_first_chunk(cursor));
}

bool page_try_split(page* self, chunk* to_split, size_t allocation_size) {
	log_trace("self = %p, allocation_size = %z <- page_try_split_chunk", self,
	          allocation_size);

	assert(!to_split->header.in_use, "Trying to split a chunk in use");

	size_t body_size = chunk_body_size(to_split);

	if (body_size < CHUNK_MIN_SIZE + allocation_size)
		return false; // Cannot split

	size_t size = CHUNK_HEADER_SIZE + allocation_size;

	chunk *next = (chunk *)((uintptr_t)to_split + size);
	chunk_init(next, size, chunk_size(to_split) - size, to_split->header.has_next,
	           false, to_split, to_split->body.list.next);

	chunk_set_size(to_split, size);
	to_split->header.has_next = true;
	to_split->body.list.next = next;
	
	return true;
}

bool page_try_fuse(page* self, chunk* c) {
	assert(!c->header.in_use, "Trying to fuse a chunk in use");

	chunk* next = chunk_next(c);

	if (!next || next->header.in_use)
		return false;

	/* TODO: use a better function */
	page_mark_in_use(self, next);

	size_t combined_size = chunk_size(c) + chunk_size(next);
	chunk_set_size(c, combined_size);
	c->header.has_next = next->header.has_next;

	return true;
}

void *page_end(page *self) {
	page_list_node *node = page_list_node_of_page(self);
	return (void *)((uintptr_t)node + self->size);
}

void page_show_chunks(page *self) {
	chunk *cursor = &self->first_chunk;

	for (; cursor; cursor = chunk_next(cursor)) {
		size_t alloc_size = chunk_body_size(cursor);
		log_trace("\t %p - %p - : %z bytes", &cursor->body.payload,
		          &cursor->body.payload + alloc_size, alloc_size);
	}
}

chunk *page_find_free(page *self, size_t size) {
	log_trace("self = %p, size = %z <- page_find_free", self, size);
	chunk *cursor = self->free;
	while (cursor && chunk_body_size(cursor) < size) {
		cursor = cursor->body.list.next;
	}
	return cursor;
}

void page_mark_in_use(page* self, chunk* c) {
	assert(!c->header.in_use, "Trying to use a busy chunk");
	c->header.in_use = true;

	chunk *next = chunk_next(c);
	if (next)
		next->header.previous_in_use = true;

	/* Remove from free list */
	chunk* next_free = c->body.list.next;
	if (next_free)
		next_free->body.list.previous = c->body.list.previous;

	chunk* previous_free = c->body.list.previous;
	if (previous_free)
		previous_free->body.list.next = next_free;
	else
		self->free = next_free;
}

void page_mark_free(page* self, chunk* c) {
	assert(c->header.in_use, "Trying to free a chunk that is already free");
	c->header.in_use = false;

	chunk *next = chunk_next(c);
	if (next)
		next->header.previous_in_use = false;

	/* Insert in free list */
	if (self->free)
		self->free->body.list.previous = c;
	c->body.list.next = self->free;
	c->body.list.previous = NULL;
	self->free = c;
}
