#include <mallok/area.h>
#include <mallok/area_list.h>
#include <mallok/chunk.h>
#include <mallok/log.h>

void area_init(area* self, size_t size) {
    log_trace("self = %p, size = %z <- area_init", self, size);

    // Initialize struct
    self->size = size;
    self->first_free_chunk = &self->first_chunk;

    // Make first chunk fill all the available space
    chunk_init(
        &self->first_chunk,
        0,
        size - offsetof(area_list_node, area.first_chunk),
        false,
        false,
        NULL,
        NULL
    );
}

void area_deinit(area* self) {
    for (chunk* cursor = &self->first_chunk; cursor;
         cursor = chunk_next(cursor)) {
        if (cursor->header.in_use)
            log_warn(
                "Unfreed allocation at %p of approximate size %z",
                &cursor->body.payload,
                chunk_body_size(cursor)
            );
    }
}

bool area_is_empty(area* self) {
    return (
        !self->first_chunk.header.in_use && !chunk_next(&self->first_chunk)
    );
}

area* area_of_first_chunk(chunk* first) {
    log_trace("area_of_first_chunk");
    area* result = (area*)((uintptr_t)first - offsetof(area, first_chunk));
    return result;
}

area* area_of_chunk(chunk* cursor) {
    while (!chunk_is_first(cursor))
        cursor = chunk_previous(cursor);
    return (area_of_first_chunk(cursor));
}

void* area_end(area* self) {
    area_list_node* node = area_list_node_of_area(self);
    return (void*)((uintptr_t)node + self->size);
}

chunk* area_find_free(area* self, size_t size) {
    log_trace("self = %p, size = %z <- area_find_free", self, size);
    chunk* previous_chunk = NULL;
    chunk* cursor = self->first_free_chunk;
    while (cursor && chunk_body_size(cursor) < size) {
        assert(cursor->body.list.previous == previous_chunk, "Incoherent chunk list");
        assert(cursor->header.in_use == false, "Unfree chunk in free list");
        previous_chunk = cursor;
        cursor = cursor->body.list.next;
    }
    return cursor;
}

void area_remove_from_free_list(area* self, chunk* c) {
    assert(!c->header.in_use, "Trying to remove a chunk that is in use");

    chunk* next_free = c->body.list.next;
    if (next_free)
        next_free->body.list.previous = c->body.list.previous;

    chunk* previous_free = c->body.list.previous;
    if (previous_free)
        previous_free->body.list.next = next_free;
    else
        self->first_free_chunk = next_free;
}

void area_mark_in_use(area* self, chunk* c) {
    assert(!c->header.in_use, "Trying to use a busy chunk");

    area_remove_from_free_list(self, c);

    c->header.in_use = true;
    chunk* next = chunk_next(c);
    if (next)
        next->header.previous_in_use = true;
}

void area_mark_free(area* self, chunk* c) {
    assert(c->header.in_use, "Trying to free a chunk that is already free");

    /* Insert in free list */
    if (self->first_free_chunk)
        self->first_free_chunk->body.list.previous = c;
    c->body.list.next = self->first_free_chunk;
    c->body.list.previous = NULL;
    self->first_free_chunk = c;

    /* Mark chunk free */
    c->header.in_use = false;
    chunk* next = chunk_next(c);
    if (next)
        next->header.previous_in_use = false;
}

bool area_try_split(area* self, chunk* to_split, size_t allocation_size) {
    log_trace(
        "self = %p, allocation_size = %z <- area_try_split_chunk",
        self,
        allocation_size
    );

    size_t body_size = chunk_body_size(to_split);

    if (body_size < CHUNK_MIN_SIZE + allocation_size)
        return false;  // Cannot split

    size_t size = CHUNK_HEADER_SIZE + allocation_size;

    chunk* next = (chunk*)((uintptr_t)to_split + size);
    chunk_init(
        next,
        size,
        chunk_size(to_split) - size,
        to_split->header.has_next,
        false,
        to_split,
        to_split->body.list.next
    );

    to_split->header.has_next = true;
    to_split->body.list.next = next;
    chunk_set_size(to_split, size);

    return true;
}

bool area_try_fuse(area* self, chunk* c) {
    chunk* next = chunk_next(c);

    if (!(next && !next->header.in_use))
        return false;

    area_remove_from_free_list(self, next);

    c->header.has_next = next->header.has_next;
    size_t combined_size = chunk_size(c) + chunk_size(next);
    chunk_set_size(c, combined_size);

    return true;
}
