#include <internals.h>
#include <log.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

void chunk_init(chunk *self, size_t previous_chunk_size, size_t size,
                bool has_next, bool previous_in_use, chunk *previous,
                chunk *next) {
	self->previous_chunk_size = previous_chunk_size;
	chunk_set_size(self, size);
	self->header.previous_in_use = previous_in_use;
	self->header.in_use = false;
	self->header.has_next = has_next;
	self->body.list.previous = previous;
	self->body.list.next = next;
}

void chunk_set_size(chunk *self, size_t size) {
	assert(!(size < sizeof(chunk)), "Invalid chunk size %z < %z", size,
	       sizeof(chunk));
	assert(size % 16 == 0, "Invalid chunk size %z mod 16 == %z", size,
	       size % 16);
	self->header.size_divided_by_16 = size / 16;
}

// Returns the size of the chunk (header + payload)
size_t chunk_size(chunk *self) { return self->header.size_divided_by_16 * 16; }

chunk *chunk_of_payload(void *payload) {
	log_trace("chunk_of_payload");
	chunk* result = (chunk *)((uintptr_t)payload - offsetof(chunk, body.payload)) ;
	assert((uintptr_t)result % alignof(chunk) == 0, "Chunk is not aligned");
	return result;
}

bool chunk_is_first(chunk *self) { return self->previous_chunk_size == 0; }

chunk *chunk_next(chunk *self) {
	if (!self->header.has_next)
		return NULL;
	return (chunk *)((uintptr_t)self + chunk_size(self));
}

chunk *chunk_previous(chunk *self) {
	if (chunk_is_first(self))
		return NULL;
	return (chunk *)((uintptr_t)self - self->previous_chunk_size);
}

size_t chunk_body_size(chunk *self) {
	return (chunk_size(self) - CHUNK_HEADER_SIZE);
}

void chunk_free(chunk *self) {
	// if (!self->header.previous_in_use) {
	// }
	// chunk *previous = chunk_previous(self);
	// if (previous && previous->header.) {
	// }
}

void chunk_try_split(chunk *self, size_t allocation_size) {
	log_trace("self = %p, allocation_size = %z <- chunk_try_split", self,
	          allocation_size);
	size_t body_size = chunk_body_size(self);

	if (body_size < CHUNK_MIN_SIZE + allocation_size)
		return; // Cannot split

	log_trace("Splitting");
	size_t size = CHUNK_HEADER_SIZE + allocation_size;

	chunk *next = (chunk *)((uintptr_t)self + size);
	chunk_init(next, size, chunk_size(self) - size, self->header.has_next,
	           false, self, self->body.list.next);


	chunk_set_size(self, size);
	self->header.has_next = true;
	self->body.list.next = next;
}

chunk *chunk_extract_from_list(chunk_list_ref *ref) {
	chunk *self = *ref;
	chunk *next = self->body.list.next;
	if (next)
		next->body.list.previous = self->body.list.previous;
	*ref = next;
	return self;
}
