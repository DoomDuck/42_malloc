#pragma once

#include <mallok/chunk.h>

#include <stddef.h>

#define PAGE_HEADER_SIZE (offsetof(page_list_node, page.first_chunk))

typedef struct {
	size_t size;
	chunk *free;
	chunk first_chunk;
} page;

/* Lifecycle */
void page_init(page *self, size_t size);
void page_deinit(page *self);
bool page_is_empty(page *self);

/* Recover a page form a chunk */
page *page_of_first_chunk(chunk *first);
page *page_of_chunk(chunk *cursor);

/* Boundaries */
void *page_end(page *self);

/* Chunk manipulation */
chunk *page_find_free(page *self, size_t allocation_size);
bool page_try_split(page *self, chunk *c, size_t allocation_size);
bool page_try_fuse(page *self, chunk *c);
void page_mark_in_use(page *self, chunk *c);
void page_mark_free(page *self, chunk *c);

/* Display */
void page_show_chunks(page *self);
