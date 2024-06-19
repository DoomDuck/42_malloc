#include <mallok/log.h>
#include <mallok/utils.h>

size_t round_up_to_multiple(size_t x, size_t factor) {
    log_trace("size = %z, factor = %z, round_up_to_multiple", x, factor);

    size_t overshot = x + (factor - 1);
    size_t result = overshot - (overshot % factor);

    log_trace("%z <- round_up_to_multiple return", result);
    return result;
}
