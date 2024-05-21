#include <log.h>
#include <internals.h>

void chunk_init(chunk* self, size_t previous_chunk_size, size_t size, bool mmaped) {
	self->previous_chunk_size = previous_chunk_size;
	// TODO: assert that: size >= sizeof(chunk)
	// TODO: check size is a multiple of 16
	self->header.size_divided_by_16 = size / 16;
	self->header.arena = false;
	self->header.mmaped = mmaped;
	self->header.previous_in_use = false;
}

size_t chunk_size(chunk* self) {
	return self->header.size_divided_by_16 * 16;
}

chunk* chunk_of_payload(void *payload) {
	log_str("chunk_of_payload\n");
	// TODO: assert alignment
	return (chunk*)((uint8_t*)payload - offsetof(chunk, body.payload));
}
