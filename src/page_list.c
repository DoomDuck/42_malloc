#include <log.h>
#include <page.h>
#include <page_list.h>
#include <sys/mman.h>
#include <unistd.h>

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

page_list_node *page_list_node_of_page(page *p) {
	return (page_list_node *)((uintptr_t)p - offsetof(page_list_node, page));
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

chunk *page_list_available_chunk(page_list *self, size_t size, page** page_of_chunk) {
	log_trace("self = %p, size = %z <- page_list_available_chunk", self, size);
	page_list_node *cursor = self->first;
	chunk *result = NULL;
	*page_of_chunk = NULL;

	for (; cursor && !result; cursor = cursor->next) {
		result = page_find_free(&cursor->page, size);
	}

	if (cursor) *page_of_chunk = &cursor->page;

	return result;
}
