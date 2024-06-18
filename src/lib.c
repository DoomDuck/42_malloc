#include <mallok/allocator.h>
#include <mallok/log.h>

void show_alloc_mem(void) {
    // TODO: use printing_instead of logging functions
    log_info("tiny:\n%P", &global_allocator.tiny);
    log_info("small:\n%P", &global_allocator.small);
    log_info("large:\n%P", &global_allocator.large);
}

void* malloc(size_t allocation_size) {
    log_trace("%z <- malloc", allocation_size);

    void* result = allocator_alloc(&global_allocator, allocation_size);

    return result;
}

void free(void* ptr) {
    log_trace("%p <- free", ptr);

    // Noop on null pointer
    if (!ptr) {
        show_alloc_mem();
        return;
    }

    allocator_dealloc(&global_allocator, ptr);
}

void* realloc(void* ptr, size_t new_size) {
    log_trace("ptr = %p, new_size = %z <- realloc", ptr, new_size);

    return allocator_realloc(&global_allocator, ptr, new_size);
}
