#include <mallok/allocator.h>
#include <mallok/area.h>
#include <mallok/area_list.h>
#include <mallok/chunk.h>
#include <mallok/log.h>
#include <mallok/mem.h>
#include <mallok/utils.h>
#include <stdalign.h>
#include <string.h>
#include <unistd.h>

void allocator_init(allocator* self) {
    // Fetch page_size
    self->page_size = getpagesize();
    log_info("%z -> page size", self->page_size);

    // Initialize mutex
    pthread_mutex_init(&self->mutex, NULL);

    // Initialize areas
    area_list_init(&self->tiny);
    area_list_init(&self->small);
    area_list_init(&self->large);

    // Preallocate some areas
    area_list_insert(
        &self->tiny,
        round_up_to_multiple(AREA_TINY_MIN_SIZE, self->page_size)
    );
    area_list_insert(
        &self->small,
        round_up_to_multiple(AREA_SMALL_MIN_SIZE, self->page_size)
    );
}

void allocator_deinit(allocator* self) {
    // Destroy mutex
    pthread_mutex_destroy(&self->mutex);

    // Deinitialize areas
    area_list_deinit(&self->tiny);
    area_list_deinit(&self->small);
    area_list_deinit(&self->large);
}

size_t allocator_area_size_for_size(allocator* self, size_t allocation_size) {
    size_t min_size = 0;
    if (allocation_size <= AREA_TINY_MAX_ALLOCATION_SIZE) {
        min_size = AREA_TINY_MIN_SIZE;
    } else if (allocation_size <= AREA_SMALL_MAX_ALLOCATION_SIZE) {
        min_size = AREA_SMALL_MIN_SIZE;
    } else {
        min_size = AREA_HEADER_SIZE + CHUNK_HEADER_SIZE + allocation_size;
    }
    return round_up_to_multiple(min_size, self->page_size);
}

area_list*
allocator_area_list_for_size(allocator* self, size_t allocation_size) {
    if (allocation_size <= AREA_TINY_MAX_ALLOCATION_SIZE)
        return &self->tiny;
    else if (allocation_size <= AREA_SMALL_MAX_ALLOCATION_SIZE)
        return &self->small;
    else
        return &self->large;
}

area_list*
allocator_area_list_for_area_size(allocator* self, size_t area_size) {
    const size_t AREA_TINY_SIZE =
        round_up_to_multiple(AREA_TINY_MIN_SIZE, self->page_size);
    const size_t AREA_SMALL_SIZE =
        round_up_to_multiple(AREA_SMALL_MIN_SIZE, self->page_size);
    if (area_size == AREA_TINY_SIZE)
        return &self->tiny;
    else if (area_size == AREA_SMALL_SIZE)
        return &self->small;
    else
        return &self->large;
}

static inline void round_to_valid_allocation_size(size_t* size) {
    if (*size < CHUNK_MIN_BODY_SIZE)
        *size = CHUNK_MIN_BODY_SIZE;
    *size = round_up_to_multiple(*size, alignof(chunk));
}

void* allocator_alloc(allocator* self, size_t allocation_size) {
    log_trace(
        "self = %p, allocation_size = %z -> allocator_alloc",
        self,
        allocation_size
    );

    round_to_valid_allocation_size(&allocation_size);

    area_list* list = allocator_area_list_for_size(self, allocation_size);
    area* a = NULL;
    chunk* c = NULL;

    if (list != &self->large)
        c = area_list_available_chunk(list, allocation_size, &a);

    if (!c) {
        size_t area_size = allocator_area_size_for_size(self, allocation_size);
        log_trace("No available chunk, creating one of size %z", area_size);
        if (!(a = area_list_insert(list, area_size))) {
            return NULL;
        }
        c = a->first_free_chunk;
    }

    area_try_split_chunk(a, c, allocation_size);

    area_mark_chunk_in_use(a, c);

    return &c->body.payload;
}

void allocator_dealloc(allocator* self, void* address) {
    chunk* c = chunk_of_payload(address);
    area* a = area_of_chunk(c);

    area_list* list = allocator_area_list_for_area_size(self, a->size);

    area_mark_chunk_free(a, c);

    area_try_fuse_chunk(a, c);

    chunk* previous = chunk_previous(c);
    if (!c->header.previous_in_use && previous)
        area_try_fuse_chunk(a, previous);

    if (area_is_empty(a) && !area_is_last(a))
        area_list_remove(list, a);
}

void* allocator_realloc(allocator* self, void* old_place, size_t new_size) {
    // On null pointer realloc is equivalent to malloc
    if (!old_place)
        return allocator_alloc(self, new_size);

    // On new_size == 0 realloc is equivalent to free
    if (new_size == 0) {
        allocator_dealloc(self, old_place);
        return NULL;
    }

    chunk* c = chunk_of_payload(old_place);
    size_t current_size = chunk_body_size(c);
    area* a = area_of_chunk(c);

    round_to_valid_allocation_size(&new_size);

    // Split current chunk
    if (new_size <= current_size) {
        area_try_split_chunk(a, c, new_size);
        return &c->body.payload;
    }

    // Try to expand to next chunk
    if (area_try_fuse_chunk(a, c) && chunk_body_size(c) >= new_size) {
        area_try_split_chunk(a, c, new_size);
        return &c->body.payload;
    }

    // Use a new chunk
    void* new_place = allocator_alloc(self, new_size);

    if (!new_place)
        return NULL;

    size_t copied_amount = current_size;
    if (new_size < current_size)
        copied_amount = new_size;
    memory_copy(new_place, old_place, copied_amount);

    allocator_dealloc(self, old_place);

    return new_place;
}

/* Multi-threaded allocation functions */
void* allocator_alloc_mt(allocator* self, size_t allocation_size) {
    pthread_mutex_lock(&self->mutex);
    void* result = allocator_alloc(self, allocation_size);
    pthread_mutex_unlock(&self->mutex);

    return result;
}

void allocator_dealloc_mt(allocator* self, void* address) {
    pthread_mutex_lock(&self->mutex);
    allocator_dealloc(self, address);
    pthread_mutex_unlock(&self->mutex);
}

void* allocator_realloc_mt(allocator* self, void* address, size_t new_size) {
    pthread_mutex_lock(&self->mutex);
    void* result = allocator_realloc(self, address, new_size);
    pthread_mutex_unlock(&self->mutex);

    return result;
}
