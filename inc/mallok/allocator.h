#pragma once

#include <mallok/area_list.h>
#include <mallok/log.h>
#include <pthread.h>
#include <stddef.h>

#define MIN_ALLOCATIONS_PER_AREA (100)

#define AREA_TINY_MAX_ALLOCATION_SIZE (128)
#define AREA_TINY_MAX_CHUNK_SIZE \
    (AREA_TINY_MAX_ALLOCATION_SIZE + CHUNK_HEADER_SIZE)
#define AREA_TINY_MIN_BODY_SIZE \
    (MIN_ALLOCATIONS_PER_AREA * AREA_TINY_MAX_CHUNK_SIZE)
#define AREA_TINY_MIN_SIZE (AREA_HEADER_SIZE + AREA_TINY_MIN_BODY_SIZE)

#define AREA_SMALL_MAX_ALLOCATION_SIZE (1024)
#define AREA_SMALL_MAX_CHUNK_SIZE \
    (AREA_SMALL_MAX_ALLOCATION_SIZE + CHUNK_HEADER_SIZE)
#define AREA_SMALL_MIN_BODY_SIZE \
    (MIN_ALLOCATIONS_PER_AREA * AREA_SMALL_MAX_CHUNK_SIZE)
#define AREA_SMALL_MIN_SIZE (AREA_HEADER_SIZE + AREA_SMALL_MIN_BODY_SIZE)

typedef struct {
    area_list tiny;
    area_list small;
    area_list large;
    size_t page_size;
    pthread_mutex_t mutex;
} allocator;

/* Lifecycle */
void allocator_init(allocator* self);
void allocator_deinit(allocator* self);

/* Allocation functions */
void* allocator_alloc(allocator* self, size_t allocation_size);
void allocator_dealloc(allocator* self, void* address);
void* allocator_realloc(allocator* self, void* address, size_t new_size);

/* Multi-threaded allocation functions */
void* allocator_alloc_mt(allocator* self, size_t allocation_size);
void allocator_dealloc_mt(allocator* self, void* address);
void* allocator_realloc_mt(allocator* self, void* address, size_t new_size);

/* List dispatch */
size_t allocator_area_size_for_size(allocator* self, size_t allocation_size);
area_list* allocator_area_list_for_area_size(allocator* self, size_t area_size);
area_list*
allocator_area_list_for_size(allocator* self, size_t allocation_size);
