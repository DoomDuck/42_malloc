#include <mallok/chunk.h>
#include <mallok/log.h>
#include <stdalign.h>

void chunk_init(
    chunk* self,
    size_t previous_chunk_size,
    size_t size,
    bool has_next,
    bool previous_in_use,
    chunk* previous,
    chunk* next
) {
    self->previous_chunk_size = previous_chunk_size;
    self->header.previous_in_use = previous_in_use;
    self->header.in_use = false;
    self->header.has_next = has_next;
    self->body.list.previous = previous;
    self->body.list.next = next;
    chunk_set_size(self, size);
}

void chunk_set_size(chunk* self, size_t size) {
    assert(
        !(size < sizeof(chunk)),
        "Invalid chunk size %z < %z",
        size,
        sizeof(chunk)
    );
    assert(
        size % 16 == 0,
        "Invalid chunk size %z mod 16 == %z",
        size,
        size % 16
    );

    self->header.size_divided_by_16 = size / 16;

    chunk* next = chunk_next(self);
    if (next)
        next->previous_chunk_size = size;
}

// Returns the size of the chunk (header + payload)
size_t chunk_size(chunk* self) {
    log_trace("self = %p -> chunk_size", self);
    return self->header.size_divided_by_16 * 16;
}

size_t chunk_body_size(chunk* self) {
    return (chunk_size(self) - CHUNK_HEADER_SIZE);
}

chunk* chunk_of_payload(void* payload) {
    log_trace("chunk_of_payload");
    chunk* result =
        (chunk*)((uintptr_t)payload - offsetof(chunk, body.payload));
    assert((uintptr_t)result % alignof(chunk) == 0, "Chunk is not aligned");
    return result;
}

bool chunk_is_first(chunk* self) {
    return self->previous_chunk_size == 0;
}

chunk* chunk_next(chunk* self) {
    if (!self->header.has_next)
        return NULL;
    return (chunk*)((uintptr_t)self + chunk_size(self));
}

chunk* chunk_previous(chunk* self) {
    if (chunk_is_first(self))
        return NULL;
    return (chunk*)((uintptr_t)self - self->previous_chunk_size);
}
