#pragma once

#include <mallok/chunk.h>
#include <stddef.h>

#define AREA_HEADER_SIZE (offsetof(area_list_node, area.first_chunk))

typedef struct {
    size_t size;
    chunk* first_free_chunk;
    chunk first_chunk;
} area;

/* Lifecycle */
void area_init(area* self, size_t size);
void area_deinit(area* self);

bool area_is_empty(area* self);
bool area_is_last(area* self);

/* Recover a area form a chunk */
area* area_of_first_chunk(chunk* first);
area* area_of_chunk(chunk* cursor);

/* Boundaries */
void* area_end(area* self);

/* Chunk manipulation */
chunk* area_find_free_chunk(area* self, size_t allocation_size);
bool area_try_split_chunk(area* self, chunk* c, size_t allocation_size);
bool area_try_fuse_chunk(area* self, chunk* c);
void area_mark_chunk_in_use(area* self, chunk* c);
void area_mark_chunk_free(area* self, chunk* c);
