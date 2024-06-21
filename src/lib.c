#include <mallok/global_state.h>
#include <mallok/print.h>
#include <unistd.h>

void show_alloc_mem(void) {
    log_trace("-> show_alloc_mem");
    pthread_mutex_lock(&state.logging_mutex);
    print_fmt(
        STDOUT_FILENO,
        "tiny:\n%l\n"
        "small:\n%l\n"
        "large:\n%l\n",
        &state.alloc.tiny,
        &state.alloc.small,
        &state.alloc.large
    );
    pthread_mutex_unlock(&state.logging_mutex);
}

void show_alloc_mem_ex(void) {
    log_trace("-> show_alloc_mem_ex");
    pthread_mutex_lock(&state.logging_mutex);
    print_fmt(
        STDOUT_FILENO,
        "tiny:\n%L\n"
        "small:\n%L\n"
        "large:\n%L\n",
        &state.alloc.tiny,
        &state.alloc.small,
        &state.alloc.large
    );
    pthread_mutex_unlock(&state.logging_mutex);
}

void* malloc(size_t allocation_size) {
    log_trace("%z -> malloc", allocation_size);

    return allocator_alloc_mt(&state.alloc, allocation_size);
}

void free(void* ptr) {
    log_trace("%p -> free", ptr);

    // Noop on null pointer
    if (!ptr)
        return;

    allocator_dealloc_mt(&state.alloc, ptr);
}

void* realloc(void* ptr, size_t new_size) {
    log_trace("ptr = %p, new_size = %z -> realloc", ptr, new_size);

    return allocator_realloc_mt(&state.alloc, ptr, new_size);
}
