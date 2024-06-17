#include <utils.h>
#include <page_type.h>
#include <allocator.h>

page_type page_type_for_allocation_size(size_t size) {
	if (size <= PAGE_TINY_MAX_ALLOCATION_SIZE)
		return page_type_tiny;
	else if (size <= PAGE_SMALL_MAX_ALLOCATION_SIZE)
		return page_type_small;
	else
		return page_type_large;
}

size_t page_type_size(page_type self, size_t allocation_size) {
	switch (self) {
	default:
	case page_type_tiny:
		return global_allocator.page_size;
	case page_type_small:
		return 3 * global_allocator.page_size;
	case page_type_large:
		return round_up_to_multiple(allocation_size + PAGE_HEADER_SIZE + CHUNK_HEADER_SIZE,
		                            global_allocator.page_size);
	}
}
