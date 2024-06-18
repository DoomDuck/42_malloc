#include <mallok/area.h>
#include <mallok/area_list.h>
#include <mallok/chunk.h>
#include <mallok/log.h>

void area_init(area* self, size_t size) {
    log_trace("self = %p, size = %z <- area_init", self, size);

    // Initialize struct
    self->size = size;
    self->free = &self->first_chunk;

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

bool area_try_split(area* self, chunk* to_split, size_t allocation_size) {
    log_trace(
        "self = %p, allocation_size = %z <- area_try_split_chunk",
        self,
        allocation_size
    );

    assert(!to_split->header.in_use, "Trying to split a chunk in use");

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
    assert(!c->header.in_use, "Trying to fuse a chunk in use");

    chunk* next = chunk_next(c);

    if (!next || next->header.in_use)
        return false;

    /* TODO: use a better function */
    area_mark_in_use(self, next);

    c->header.has_next = next->header.has_next;
    size_t combined_size = chunk_size(c) + chunk_size(next);
    chunk_set_size(c, combined_size);

    return true;
}

void* area_end(area* self) {
    area_list_node* node = area_list_node_of_area(self);
    return (void*)((uintptr_t)node + self->size);
}

void area_show_chunks(area* self) {
    chunk* cursor = &self->first_chunk;

    for (; cursor; cursor = chunk_next(cursor)) {
        size_t alloc_size = chunk_body_size(cursor);
        log_trace(
            "\t %p - %p - : %z bytes",
            &cursor->body.payload,
            &cursor->body.payload + alloc_size,
            alloc_size
        );
    }
}

chunk* area_find_free(area* self, size_t size) {
    log_trace("self = %p, size = %z <- area_find_free", self, size);
    chunk* cursor = self->free;
    while (cursor && chunk_body_size(cursor) < size) {
        cursor = cursor->body.list.next;
    }
    return cursor;
}

void area_mark_in_use(area* self, chunk* c) {
    assert(!c->header.in_use, "Trying to use a busy chunk");
    c->header.in_use = true;

    chunk* next = chunk_next(c);
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

void area_mark_free(area* self, chunk* c) {
    assert(c->header.in_use, "Trying to free a chunk that is already free");
    c->header.in_use = false;

    chunk* next = chunk_next(c);
    if (next)
        next->header.previous_in_use = false;

    /* Insert in free list */
    if (self->free)
        self->free->body.list.previous = c;
    c->body.list.next = self->free;
    c->body.list.previous = NULL;
    self->free = c;
}
