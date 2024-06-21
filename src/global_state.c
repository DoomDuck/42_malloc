#include <mallok/global_state.h>
#include <stdlib.h>
#include <unistd.h>

#include "mallok/allocator.h"
#include "pthread.h"

global_state state = {
    .initialized = false,
    .logging_level = log_level_error,
    .alloc =
        {
            .tiny = {.first = NULL},
            .small = {.first = NULL},
            .large = {.first = NULL},
        },
};

void global_state_assert_init(void) {
    if (state.initialized)
        return;

    // Initialize logging system
    pthread_mutex_init(&state.logging_mutex, NULL);

    const char* log_env_var = getenv("MALLOK_LOG");
    state.logging_level = log_level_error;
    if (log_env_var)
        state.logging_level = log_level_from_name(log_env_var);

    allocator_init(&state.alloc);

    // All initialized
    state.initialized = true;
}

void global_state_assert_deinit(void) {
    if (!state.initialized)
        return;

    // Free remaning mapped regions and display warning
    allocator_deinit(&state.alloc);

    // Stop logging
    state.logging_level = log_level_off;
    pthread_mutex_destroy(&state.logging_mutex);
}
