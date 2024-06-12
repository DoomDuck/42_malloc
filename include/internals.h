#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Utils
 */

size_t round_up_to_multiple(size_t x, size_t factor);

/*
 * Chunk
 */

#define CHUNK_MIN_SIZE (sizeof(chunk))
#define CHUNK_HEADER_SIZE (offsetof(chunk, body))
#define CHUNK_MIN_BODY_SIZE (sizeof(chunk_body))

typedef struct {
	size_t size_divided_by_16 : 8 * sizeof(size_t) - 3;
	// For previous
	bool arena : 1;
	// TODO: do I keep it ?
	bool mmaped : 1;
	bool previous_in_use : 1;
} chunk_header;

typedef struct s_chunk* chunk_list_ref;

typedef union {
	uint8_t payload;
	struct {
		chunk_list_ref previous;
		chunk_list_ref next;
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
void chunk_set_size(chunk *self, size_t size);
size_t chunk_body_size(chunk *self);
chunk *chunk_next_unchecked(chunk *self);
void chunk_try_split(chunk *self, size_t allocation_size);
chunk* chunk_extract_from_list(chunk_list_ref *ref);

/*
 * Page
 */

typedef enum {
	page_type_tiny,
	page_type_small,
	page_type_large,
} page_type;

typedef struct {
	size_t size;
	chunk *first_free;
	chunk first_chunk;
} page;

struct page_list_node;
typedef struct s_page_list_node page_list_node;

struct s_page_list_node {
	page_list_node *previous;
	page_list_node *next;
	page page;
};

typedef struct {
	page_list_node *first;
	size_t length;
	size_t free_count;
} page_list;

void page_list_init(page_list *self);
void page_list_deinit(page_list *self);
page_list_node *page_list_node_of_page(page *node);

void page_list_show(page_list *self);

page *page_list_insert(page_list *self, size_t page_size);
void page_list_remove(page_list *self, page *page);

size_t page_type_size(page_type self, size_t required_size);
page_type page_type_for_allocation_size(size_t allocation_size);

void page_init(page *self, size_t size);
void page_deinit(page *self);

void *page_end(page *self);

page *page_of_first_chunk(chunk *first);
chunk_list_ref *page_list_available_chunk(page_list *self, size_t size);

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
void *allocator_alloc(allocator *self, size_t allocation_size);
page_list *allocator_page_list(allocator *self, page_type type);

// TODO: set the correct prority
void __attribute__((constructor(101))) global_allocator_init(void);

void global_allocator_init(void);

