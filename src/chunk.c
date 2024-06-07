#include <internals.h>
#include <log.h>

void chunk_init(chunk *self, size_t previous_chunk_size, size_t size,
                bool mmaped) {
	self->previous_chunk_size = previous_chunk_size;
	// TODO: assert that: size >= sizeof(chunk)
	// TODO: check size is a multiple of 16
	self->header.size_divided_by_16 = size / 16;
	self->header.arena = false;
	self->header.mmaped = mmaped;
	self->header.previous_in_use = false;
}

size_t chunk_size(chunk *self) { return self->header.size_divided_by_16 * 16; }

chunk *chunk_of_payload(void *payload) {
	log_fmt("chunk_of_payload\n");
	// TODO: assert alignment
	return (chunk *)((uint8_t *)payload - offsetof(chunk, body.payload));
}

bool chunk_is_first(chunk *self) { return self->previous_chunk_size == 0; }

chunk *chunk_next(chunk *self) {
	if (chunk_is_first(self))
		return NULL;
	return (chunk *)((size_t)self - self->previous_chunk_size);
}

chunk *chunk_previous(chunk *self) {
	if (chunk_is_first(self))
		return NULL;
	return (chunk *)((size_t)self - self->previous_chunk_size);
}

void chunk_free(chunk *self) {
	// if (!self->header.previous_in_use) {
	// }
	// chunk *previous = chunk_previous(self);
	// if (previous && previous->header.) {
	// }
}
