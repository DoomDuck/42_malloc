#include <internals.h>
#include <log.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_TINY_MAX_ALLOCATION_SIZE 1024
#define PAGE_SMALL_MAX_ALLOCATION_SIZE (1024 * 1024)

page_type page_type_for_allocation_size(size_t size) {
	if (size <= PAGE_TINY_MAX_ALLOCATION_SIZE)
		return page_type_tiny;
	else if (size <= PAGE_SMALL_MAX_ALLOCATION_SIZE)
		return page_type_small;
	else
		return page_type_large;
}

size_t round_up_to_multiple(size_t x, size_t factor) {
	log_trace("size = %z, factor = %z, round_up_to_multiple", x, factor);

	size_t overshot = x + (factor - 1);

	log_trace("%z <- round_up_to_multiple return",
	          overshot - overshot % factor);
	return overshot - overshot % factor;
}

size_t page_type_size(page_type self, size_t allocation_size) {
	switch (self) {
	default:
	case page_type_tiny:
		return global_allocator.page_size;
	case page_type_small:
		return 3 * global_allocator.page_size;
	case page_type_large:
		return round_up_to_multiple(allocation_size,
		                            global_allocator.page_size);
	}
}

void page_list_init(page_list *self) {
	self->first = NULL;
	self->length = 0;
	self->free_count = 0;
}

page *page_list_insert(page_list *self, size_t page_size) {

	// Ask for new page
	page_list_node *node =
	    mmap(NULL,                   // To address is required
	         page_size,              // Size of the page to request
	         PROT_READ | PROT_WRITE, // Allow reading and writing
	         MAP_ANONYMOUS           // Do not map of filesystem
	             | MAP_PRIVATE,      // Do not share with other processes
	         -1, 0                   // unused filesystem specific options
	    );

	if (node == MAP_FAILED) {
		// Could not create page
		log_error("map failed: %x");
		return NULL;
	}

	log_trace("%p <- mapped address", node);

	if (self->first)
		self->first->previous = node;
	node->next = self->first;
	node->previous = NULL;
	self->first = node;
	page_init(&node->page, page_size);

	log_trace("%p <- mapped address", node);
	return &node->page;
}

void page_list_remove(page_list *self, page *page) {
	log_trace("self = %p, page = %p <- page_list_remove", self, page);

	page_list_node *node = page_list_node_of_page(page);

	if (self->first == node) {
		self->first = node->next;
	} else {
		node->previous->next = node->next;
	}

	if (node->next)
		node->next->previous = node->previous;

	if (munmap(node, page->size)) {
		log_error("%e <- munmap error");
		exit(1);
	}
}

page_list_node* page_list_node_of_page(page* p) {
	return (page_list_node*)((uintptr_t)p - offsetof(page_list_node, page));
}

void page_init(page *self, size_t size) {
	log_trace("self = %p, size = %z <- page_init", self, size);

	// Initialize struct
	self->size = size;
	self->first_free = &self->first_chunk;

	// Make first chunk fill all the available space
	// TODO: check if this is a large page
	chunk_init(&self->first_chunk, 0,
	           size - offsetof(page_list_node, page.first_chunk), true);
}

chunk_list_ref *page_find_free_chunk(page *self, size_t size) {
	log_trace("self = %p, size = %z <- page_find_free_chunk", self, size);
	chunk_list_ref *cursor = &self->first_free;
	while (*cursor && chunk_body_size(*cursor) < size) {
		cursor = &(*cursor)->body.list.next;
	}
	return *cursor ? cursor : NULL;
}

page *page_of_first_chunk(chunk *first) {
	log_trace("page_of_first_chunk");
	// TODO: assert alignment
	return (page *)((uintptr_t)first - offsetof(page, first_chunk));
}

void *page_end(page *self) {
	page_list_node *node = page_list_node_of_page(self);
	return (void *)((uintptr_t)node + self->size);
}

void page_show_chunks(page *self) {
	void *end = page_end(self);
	chunk *cursor = &self->first_chunk;

	for (; (void *)cursor < end; cursor = chunk_next_unchecked(cursor)) {
		size_t alloc_size = chunk_body_size(cursor);
		log_trace("\t %p - %p - : %z bytes", &cursor->body.payload,
		          &cursor->body.payload + alloc_size, alloc_size);
	}
}

void page_list_show(page_list *self) {
	log_trace("self = %p <- page_list_show", self);
	// The first node is not a real node
	page_list_node *cursor = self->first;

	log_trace("\t first = %p", self->first);

	for (; cursor; cursor = cursor->next) {
		page_show_chunks(&cursor->page);
	}
}

chunk_list_ref *page_list_available_chunk(page_list *self, size_t size) {
	log_trace("self = %p, size = %z <- page_list_available_chunk", self, size);
	page_list_node *cursor = self->first;
	chunk_list_ref *result = NULL;

	for (; cursor && !result; cursor = cursor->next) {
		result = page_find_free_chunk(&cursor->page, size);
	}

	return result;
}
