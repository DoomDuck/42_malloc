#pragma once

#include <mallok/area_list.h>
#include <mallok/log.h>
#include <pthread.h>
#include <stddef.h>

#define AREA_TINY_MAX_ALLOCATION_SIZE (128)
#define AREA_SMALL_MAX_ALLOCATION_SIZE (1024)
#define MIN_ALLOCATIONS_PER_AREA (100)
#define AREA_TINY_SIZE \
    (MIN_ALLOCATIONS_PER_AREA * AREA_TINY_MAX_ALLOCATION_SIZE)
#define AREA_SMALL_SIZE \
    (MIN_ALLOCATIONS_PER_AREA * AREA_SMALL_MAX_ALLOCATION_SIZE)

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

/* Initialize allocator if not allready */
void allocator_assert_init(allocator* self);

/* Allocation functions */
void* allocator_alloc(allocator* self, size_t allocation_size);
void allocator_dealloc(allocator* self, void* address);
void* allocator_realloc(allocator* self, void* address, size_t new_size);

/* Allocator locking functions */
void* allocator_maybe_lock(allocator* self);
void* allocator_maybe_unlock(allocator* self);

/* Multi-threaded allocation functions */
void* allocator_alloc_mt(allocator* self, size_t allocation_size);
void allocator_dealloc_mt(allocator* self, void* address);
void* allocator_realloc_mt(allocator* self, void* address, size_t new_size);

/* List dispatch */
size_t allocator_area_size_for_size(allocator* self, size_t allocation_size);
area_list*
allocator_area_list_for_size(allocator* self, size_t allocation_size);
