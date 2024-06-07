#include <internals.h>
#include <log.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

allocator global_allocator;

void allocator_init(allocator *self) {
	self->page_size = getpagesize();
	log_fmt("%z <- page size\n", self->page_size);
	// TODO: initalize minimal amount of pages
	self->tiny = (page_list_node){NULL, NULL};
	self->small = (page_list_node){NULL, NULL};
	self->large = (page_list_node){NULL, NULL};
}

page_list_node *allocator_page_list(allocator *self, page_type type) {
	switch (type) {
	default:
	case page_type_tiny:
		return &self->tiny;
	case page_type_small:
		return &self->small;
	case page_type_large:
		return &self->large;
	}
}

void global_allocator_init(void) { allocator_init(&global_allocator); }
