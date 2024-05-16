#include <errno.h>
#include <malloc.h>
#include <internals.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#define LOG_FD 2

typedef int fd;

size_t string_length(const char* s) {
	size_t length = 0;
	while (s[length])
		length++;
	return length;
}

void write_string(fd output, const char* s) {
	write(output, s, string_length(s));
}

void print_size_t(fd output, size_t n) {
	char buffer[20];
	char* cursor = &buffer[sizeof(buffer)];

	*--cursor = '\0';
	do {
		*--cursor = '0' + (n % 10);
		n /= 10;
	} while (n > 0);

	write_string(output, cursor);
}

void print_pointer(fd output, void* p) {
	char buffer[26];
	char* cursor = &buffer[sizeof(buffer)];

	size_t n = (size_t)p;

	*--cursor = '\0';
	do {
		size_t rest = (n % 16);
		if (rest < 10) *--cursor = '0' + rest;
		else *--cursor = 'a' + (rest - 10);
		n /= 16;
	} while (n > 0);
	
	// Add prefix
	*--cursor = 'x';
	*--cursor = '0';

	write_string(output, cursor);
}

allocator global_allocator;

void allocator_init(allocator* self) {
	self->page_size = getpagesize();
	print_size_t(LOG_FD, self->page_size);
	write_string(LOG_FD, " <- page size");
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

void __attribute__((constructor)) allocator_state_global(void);

void allocator_state_global(void) {
	allocator_init(&global_allocator);
}

void chunk_init(chunk* self, size_t previous_chunk_size, size_t size, bool mmaped) {
	self->previous_chunk_size = previous_chunk_size;
	// TODO: assert that: size >= sizeof(chunk)
	// TODO: check size is a multiple of 16
	self->header.size_divided_by_16 = size / 16;
	self->header.arena = false;
	self->header.mmaped = mmaped;
	self->header.previous_in_use = false;
}

size_t chunk_size(chunk* self) {
	return self->header.size_divided_by_16 * 16;
}

chunk* chunk_of_payload(void *payload) {
	write_string(LOG_FD, "chunk_of_payload\n");
	// TODO: assert alignment
	return (chunk*)((uint8_t*)payload - offsetof(chunk, body.payload));
}

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
	write_string(LOG_FD, "round_up_to_multiple\n");
	print_size_t(LOG_FD, factor);
	write_string(LOG_FD, " <- round_up_to_multiple factor\n");

	size_t overshot = x + (factor - 1);

	print_size_t(LOG_FD, overshot - overshot % factor);
	write_string(LOG_FD, " <- round_up_to_multiple return\n");
	return overshot - overshot % factor;
}

size_t page_type_size(page_type self, size_t allocation_size) {
	switch (self) {
	default: case page_type_tiny:
		return global_allocator.page_size;
	case page_type_small:
		return 3 * global_allocator.page_size;
	case page_type_large:
		return round_up_to_multiple(allocation_size, getpagesize()); // global_allocator.page_size);
    }
}

page* page_new(page_type type, size_t allocation_size) {
	write_string(LOG_FD, "page_new\n");

	// For now only
	size_t size = page_type_size(type, allocation_size);

	// Ask for new page
	page* p = mmap(
		NULL, // To address is required
		size, // Size of the page to request
		PROT_READ | PROT_WRITE, // Allow reading and writing
		MAP_ANONYMOUS // Do not map of filesystem
		| MAP_PRIVATE, // Do not share with other processes
		-1, 0 // unused filesystem specific options
	);

	if (p == MAP_FAILED) {
		// TODO: log
		// Could tot create page
		write_string(LOG_FD, ">> map failed\n");
		return NULL;
	}

	print_pointer(LOG_FD, p);
	write_string(LOG_FD, " <- mapped address\n");

	// Insert new page
	page_list_node* list = allocator_page_list(&global_allocator, type);
	page_init(p, size, list, list->next);

	return p;
}

void page_init(page* self, size_t size, page_list_node* previous, page* next) {
	write_string(LOG_FD, "page_init\n");

	// Initialize struct
	self->size = size;
	self->node.previous = previous;
	self->node.next = next;
	self->first_free = &self->first_chunk;

	// Make first chunk fill all the available space
	// TODO: check if this is a large page
	chunk_init(&self->first_chunk, 0, size - offsetof(page, first_chunk), true);

	// Update neighbouring links
	previous->next = self;
	if (next)
		next->node.previous = &self->node;
}

void page_deinit(page* self) {
	write_string(LOG_FD, "page_deinit\n");

	print_pointer(LOG_FD, self);
	write_string(LOG_FD, " <- deinit address, ");
	print_size_t(LOG_FD, self->size);
	write_string(LOG_FD, " <- size\n");

	write_string(LOG_FD, "Relinking: Start\n");

	// Remove page from page_list
	self->node.previous->next = self->node.next;
	if (self->node.next)
		self->node.next->node.previous = self->node.previous;

	write_string(LOG_FD, "Relinking: End\n");

	if (munmap(self, self->size)) {
		// TODO: log error
		write_string(LOG_FD, strerror(errno));
		write_string(LOG_FD, " <- munmap error\n");
		exit(1);
	}
}

page* page_of_first_chunk(chunk* first) {
	write_string(LOG_FD, "page_of_first_chunk\n");
	// TODO: assert alignment
	return (page*)((uint8_t*)first - offsetof(page, first_chunk));
}

void* malloc(size_t allocation_size) {
	print_size_t(LOG_FD, allocation_size);
	write_string(LOG_FD, " <- malloc\n");

	// For now only use large page algorithm
	// TODO: Lookup good page type
	// 	   page_type type = page_type_for_allocation_size(allocation_size);
	page_type type = page_type_large;

	page* p = page_new(type, allocation_size);

	if (!p) return NULL;

	print_pointer(LOG_FD, &p->first_chunk.body.payload);
	write_string(LOG_FD, " <- malloc return\n");
	return &p->first_chunk.body.payload;
}

void free(void* ptr) {
	print_pointer(LOG_FD, ptr);
	write_string(LOG_FD, " <- free\n");

	// Noop on null pointer
	if (!ptr) return;

	chunk* c = chunk_of_payload(ptr);
	// TODO: change later on
	page* p = page_of_first_chunk(c);

	// Delete page
	page_deinit(p);
}

void memory_copy(void* destination, const void* source, size_t count) {
	uint8_t* destination_array = destination;
	const uint8_t* source_array = source;
	for (size_t i = 0; i < count; ++i)
		destination_array[i] = source_array[i];
}

void* realloc(void* ptr, size_t new_size) {
	write_string(LOG_FD, "ptr = ");
	print_pointer(LOG_FD, ptr);
	write_string(LOG_FD, ", new_size = ");
	print_size_t(LOG_FD, new_size);
	write_string(LOG_FD, " <- realloc\n");

	// On null ptr realloc is equivalent to malloc
	if (!ptr) return malloc(new_size);

	chunk* c = chunk_of_payload(ptr);

	size_t previous_size = chunk_size(c);

	void* new_place = malloc(new_size);
	
	size_t copied_amount = previous_size;
	if (previous_size > new_size) copied_amount = new_size;
	memory_copy(new_place, ptr, copied_amount);

	free(ptr);

	return new_place;
}
