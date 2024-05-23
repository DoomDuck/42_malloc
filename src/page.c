#include <internals.h>
#include <log.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

/* TODO: assess if I can use them */
#include <errno.h>
#include <string.h>

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
  log_str("round_up_to_multiple\n");
  log_size_t(factor);
  log_str(" <- round_up_to_multiple factor\n");

  size_t overshot = x + (factor - 1);

  log_size_t(overshot - overshot % factor);
  log_str(" <- round_up_to_multiple return\n");
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
    return round_up_to_multiple(allocation_size, global_allocator.page_size);
  }
}

page *page_new(page_type type, size_t allocation_size) {
  log_str("page_new\n");

  // For now only
  size_t size = page_type_size(type, allocation_size);

  // Ask for new page
  page *p = mmap(NULL,                   // To address is required
                 size,                   // Size of the page to request
                 PROT_READ | PROT_WRITE, // Allow reading and writing
                 MAP_ANONYMOUS           // Do not map of filesystem
                     | MAP_PRIVATE,      // Do not share with other processes
                 -1, 0                   // unused filesystem specific options
  );

  if (p == MAP_FAILED) {
    // TODO: log
    // Could tot create page
    log_str(">> map failed\n");
    return NULL;
  }

  log_pointer(p);
  log_str(" <- mapped address\n");

  // Insert new page
  page_list_node *list = allocator_page_list(&global_allocator, type);
  page_init(p, size, list, list->next);

  return p;
}

void page_init(page *self, size_t size, page_list_node *previous,
               page_list_node *next) {
  log_str("page_init\n");

  // Initialize struct
  self->size = size;
  self->node.previous = previous;
  self->node.next = next;
  self->first_free = &self->first_chunk;

  // Make first chunk fill all the available space
  // TODO: check if this is a large page
  chunk_init(&self->first_chunk, 0, size - offsetof(page, first_chunk), true);

  // Update neighbouring links
  previous->next = &self->node;
  if (next)
    next->previous = &self->node;
}

void page_deinit(page *self) {
  log_str("page_deinit\n");

  log_pointer(self);
  log_str(" <- deinit address, ");
  log_size_t(self->size);
  log_str(" <- size\n");

  // Remove page from page_list
  self->node.previous->next = self->node.next;
  if (self->node.next)
    self->node.next->previous = self->node.previous;

  if (munmap(self, self->size)) {
    // TODO: log error
    log_str(strerror(errno));
    log_str(" <- munmap error\n");
    exit(1);
  }
}

chunk *page_find_free_chunk(page *self, size_t size) {
  chunk *cursor = self->first_free;
  while (cursor && chunk_size(cursor) < size)
    cursor = cursor->body.list.next;
  return cursor;
}

page *page_of_first_chunk(chunk *first) {
  log_str("page_of_first_chunk\n");
  // TODO: assert alignment
  return (page *)((uint8_t *)first - offsetof(page, first_chunk));
}

page *page_of_list_node(page_list_node *node) {
  // TODO: assert node->previous != NULL
  return (page *)((uint8_t *)node - offsetof(page, node));
}

void page_show_chunks(page *self) {
  uint8_t *page_end = ((uint8_t *)self + self->size);
  uint8_t *cursor = (uint8_t *)&self->first_chunk;

  while (cursor < page_end) {
    chunk *c = (chunk *)cursor;
    size_t size = chunk_size(c);
    size_t alloc_size = size - offsetof(chunk, body);
    log_pointer(&c->body.payload);
    log_str(" - ");
    log_pointer(&c->body.payload + alloc_size);
    log_str(" : ");
    log_size_t(alloc_size);
    log_str(" bytes\n");
  }
}

void page_list_show(page_list *self) {
  // The first node is not a real node
  page_list_node *cursor = self->next;

  for (; cursor; cursor = cursor->next) {
    page_show_chunks(page_of_list_node(cursor));
  }
}

chunk *page_list_find_free_chunk(page_list * self, size_t size) {
  // The first node is not a real node
  page_list_node *cursor = self->next;
  chunk *result = NULL;

  for (; cursor && !result; cursor = cursor->next) {
    page *p = page_of_list_node(cursor);
    result = page_find_free_chunk(p, size);
  }
  return result;
}
