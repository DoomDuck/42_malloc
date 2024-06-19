#include <mallok/allocator.h>
#include <mallok/log.h>
#include <mallok/print.h>
#include <unistd.h>

void show_alloc_mem(void) {
    print_fmt(
        STDOUT_FILENO,
        "tiny:\n%A\n"
        "small:\n%A\n"
        "large:\n%A\n",
        &global_allocator.tiny,
        &global_allocator.small,
        &global_allocator.large
    );
}

void* malloc(size_t allocation_size) {
    log_trace("%z <- malloc", allocation_size);

    return allocator_alloc_mt(&global_allocator, allocation_size);
}

void free(void* ptr) {
    log_trace("%p <- free", ptr);

    // Noop on null pointer
    if (!ptr)
        return;

    allocator_dealloc_mt(&global_allocator, ptr);
}

void* realloc(void* ptr, size_t new_size) {
    log_trace("ptr = %p, new_size = %z <- realloc", ptr, new_size);

    return allocator_realloc_mt(&global_allocator, ptr, new_size);
}
