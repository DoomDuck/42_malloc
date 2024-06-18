#pragma once

#include <mallok/page.h>

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

/* Lifecycle */
void page_list_init(page_list *self);
void page_list_deinit(page_list *self);

/* Recover */
page_list_node *page_list_node_of_page(page *node);

/* List manipulations */
page *page_list_insert(page_list *self, size_t page_size);
void page_list_remove(page_list *self, page *page);

/* Find available chunk */
chunk *page_list_available_chunk(page_list *self, size_t size,
                                 page **page_of_chunk);

/* Display */
void page_list_show(page_list *self);
