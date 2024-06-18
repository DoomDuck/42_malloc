#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CHUNK_MIN_SIZE (sizeof(chunk))
#define CHUNK_HEADER_SIZE (offsetof(chunk, body))
#define CHUNK_MIN_BODY_SIZE (sizeof(chunk_body))

typedef struct {
    size_t size_divided_by_16 : 8 * sizeof(size_t) - 3;
    bool previous_in_use : 1;
    bool in_use : 1;
    bool has_next : 1;
} chunk_header;

typedef struct s_chunk chunk;

typedef union {
    uint8_t payload;

    struct {
        chunk* previous;
        chunk* next;
    } list;
} __attribute__((aligned(16))) chunk_body;

typedef struct s_chunk {
    size_t previous_chunk_size;
    chunk_header header;
    chunk_body body;
} chunk;

void chunk_init(
    chunk* self,
    size_t previous_chunk_size,
    size_t size,
    bool has_next,
    bool previous_in_use,
    chunk* previous,
    chunk* next
);
chunk* chunk_of_payload(void* payload);
size_t chunk_size(chunk* self);
bool chunk_is_first(chunk* self);

void chunk_set_size(chunk* self, size_t size);
size_t chunk_body_size(chunk* self);
chunk* chunk_next(chunk* self);
void chunk_try_fuse(chunk* self);
chunk* chunk_extract_from_list(chunk* ref);
chunk* chunk_previous(chunk* self);
