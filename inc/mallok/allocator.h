#pragma once

#include <mallok/log.h>
#include <mallok/page_list.h>
#include <mallok/page_type.h>

#include <stddef.h>

typedef struct {
	size_t page_size;
	log_level logging_level;
	page_list tiny;
	page_list small;
	page_list large;
} allocator;

/* Lifecycle */
void allocator_init(allocator *self);
void allocator_deinit(allocator *self);

/* Allocation function */
void *allocator_alloc(allocator *self, size_t allocation_size);
void allocator_dealloc(allocator *self, void *address);
void *allocator_realloc(allocator *self, void *address, size_t new_size);

/* Get page list of type */
page_list *allocator_page_list(allocator *self, page_type type);

/* Global allocator */
extern allocator global_allocator;

// TODO: set the correct prority
void __attribute__((constructor(101))) global_allocator_init(void);
