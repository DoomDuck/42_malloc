#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Chunk
 */

typedef struct {
	size_t size_divided_by_16 : 8 * sizeof(size_t) - 3;
	// For previous
	bool arena : 1;
	bool mmaped : 1;
	bool previous_in_use : 1;
} chunk_header;

typedef union {
	uint8_t payload;
	struct {
		struct s_chunk *previous;
		struct s_chunk *next;
	} list;
} __attribute__((aligned(16))) chunk_body;

typedef struct s_chunk {
	size_t previous_chunk_size;
	chunk_header header;
	chunk_body body;
} chunk;

void chunk_init(chunk *self, size_t previous_chunk_size, size_t size,
                bool mmaped);
chunk *chunk_of_payload(void *payload);
size_t chunk_size(chunk *self);

/*
 * Page
 */

typedef enum {
	page_type_tiny,
	page_type_small,
	page_type_large,
} page_type;

typedef struct s_page page;

typedef struct s_page_list_node {
	struct s_page_list_node *previous;
	struct s_page_list_node *next;
} page_list_node;

// Name reserved for the first node of a list
typedef page_list_node page_list;

struct s_page {
	page_list_node node;
	size_t size;
	chunk *first_free;
	chunk first_chunk;
};

size_t page_type_size(page_type self, size_t required_size);
page_type page_type_for_allocation_size(size_t allocation_size);

page *page_new(page_type type, size_t allocation_size);
void page_init(page *self, size_t size, page_list_node *previous,
               page_list_node *next);
void page_deinit(page *self);
page *page_of_first_chunk(chunk *first);
page *page_of_list_node(page_list_node *node);
void page_list_show(page_list *self);

/*
 * Allocator
 */
typedef struct {
	size_t page_size;
	page_list tiny;
	page_list small;
	page_list large;
} allocator;

extern allocator global_allocator;

void allocator_init(allocator *self);
page_list_node *allocator_page_list(allocator *self, page_type type);

// TODO: set the correct prority
void __attribute__((constructor(101))) global_allocator_init(void);

void global_allocator_init(void);
