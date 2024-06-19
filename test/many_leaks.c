#include <stdlib.h>

void* unoptimized_malloc(size_t size) {
    return malloc(size);
}

int main(void) {
    for (size_t i = 0; i < 1024; ++i)
        unoptimized_malloc(1024);
}
