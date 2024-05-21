#include <log.h>
#include <internals.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

allocator global_allocator;

void allocator_init(allocator* self) {
	self->page_size = getpagesize();
	log_size_t(self->page_size);
	log_str(" <- page size");
	// TODO: initalize minimal amount of pages
	self->tiny = (page_list_node){NULL, NULL};
	self->small = (page_list_node){NULL, NULL};
	self->large = (page_list_node){NULL, NULL};
}

page_list_node* allocator_page_list(allocator *self, page_type type) {
	switch (type) {
	default: case page_type_tiny:
		return &self->tiny;
	case page_type_small:
		return &self->small;
	case page_type_large:
		return &self->large;
	}
}

// TODO: set the correct prority
// void __attribute__((constructor)) allocator_state_global(void);

void allocator_state_global(void) {
	allocator_init(&global_allocator);
}
