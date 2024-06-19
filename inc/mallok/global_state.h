#pragma once

#include <stdbool.h>
#include <mallok/log.h>
#include <mallok/allocator.h>

typedef struct {
    bool initialized;

    /* Global allocator */
    allocator alloc;

    /* Logging */
    log_level logging_level;
    pthread_mutex_t logging_mutex;
} global_state;

extern global_state state;

void __attribute__((constructor(101))) global_state_assert_init(void);
void __attribute__((destructor(101))) global_state_assert_deinit(void);
