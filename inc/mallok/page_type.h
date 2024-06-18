#pragma once

#include <stddef.h>

#define PAGE_TINY_MAX_ALLOCATION_SIZE 128
#define PAGE_SMALL_MAX_ALLOCATION_SIZE 1024

typedef enum {
	page_type_tiny,
	page_type_small,
	page_type_large,
} page_type;

size_t page_type_size(page_type self, size_t required_size);
page_type page_type_for_allocation_size(size_t allocation_size);
