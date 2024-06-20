#include <mallok/area.h>
#include <mallok/area_list.h>
#include <mallok/log.h>
#include <sys/mman.h>
#include <unistd.h>

void area_list_init(area_list* self) {
    self->first = NULL;
}

void area_list_deinit(area_list* self) {
    while (self->first) {
        area_list_remove(self, &self->first->area);
        ;
    }
}

area* area_list_insert(area_list* self, size_t area_size) {
    // Ask for new area
    area_list_node* node = mmap(
        NULL,  // To address is required
        area_size,  // Size of the area to request
        PROT_READ | PROT_WRITE,  // Allow reading and writing
        MAP_ANONYMOUS  // Do not map of filesystem
            | MAP_PRIVATE,  // Do not share with other processes
        -1,
        0  // unused filesystem specific options
    );

    if (node == MAP_FAILED) {
        // Could not create area
        log_error("map failed: %x");
        return NULL;
    }

    log_trace("%p <- mapped address", node);

    if (self->first)
        self->first->previous = node;
    node->next = self->first;
    node->previous = NULL;
    self->first = node;
    area_init(&node->area, area_size);

    log_trace("%p <- mapped address", node);
    return &node->area;
}

void area_list_remove(area_list* self, area* area) {
    log_trace("self = %p, area = %p <- area_list_remove", self, area);

    area_deinit(area);

    area_list_node* node = area_list_node_of_area(area);

    if (self->first == node) {
        self->first = node->next;
    } else {
        node->previous->next = node->next;
    }

    if (node->next)
        node->next->previous = node->previous;

    if (munmap(node, area->size))
        log_error("%e <- munmap error");
}

area_list_node* area_list_node_of_area(area* a) {
    return (area_list_node*)((uintptr_t)a - offsetof(area_list_node, area));
}

chunk*
area_list_available_chunk(area_list* self, size_t size, area** area_of_chunk) {
    log_trace("self = %p, size = %z <- area_list_available_chunk", self, size);
    area_list_node* cursor = self->first;
    chunk* result = NULL;
    *area_of_chunk = NULL;

    for (; cursor; cursor = cursor->next) {
        if ((result = area_find_free(&cursor->area, size))) {
            *area_of_chunk = &cursor->area;
            return result;
        }
    }

    return NULL;
}
