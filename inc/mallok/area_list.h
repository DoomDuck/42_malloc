#pragma once

#include <mallok/area.h>

struct area_list_node;
typedef struct s_area_list_node area_list_node;

struct s_area_list_node {
    area_list_node* previous;
    area_list_node* next;
    area area;
};

typedef struct {
    area_list_node* first;
    size_t length;
    size_t free_count;
} area_list;

/* Lifecycle */
void area_list_init(area_list* self);
void area_list_deinit(area_list* self);

/* Recover */
area_list_node* area_list_node_of_area(area* node);

/* List manipulations */
area* area_list_insert(area_list* self, size_t area_size);
void area_list_remove(area_list* self, area* area);

/* Find available chunk */
chunk*
area_list_available_chunk(area_list* self, size_t size, area** area_of_chunk);

/* Display */
void area_list_show(area_list* self);
