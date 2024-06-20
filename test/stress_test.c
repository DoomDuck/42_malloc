#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ALLOC_COUNT 1000
#define REALLOC_COUNT 100
#define TINY_ALLOC_MAX_SIZE 128
#define SMALL_ALLOC_MAX_SIZE 1024
#define LARGE_ALLOC_MAX_SIZE 10240

typedef struct {
    char* data;
    size_t size;
    unsigned int seed;
} allocation_info;

bool write_all(int output, const void* bytes, size_t count) {
    while (count != 0) {
        ssize_t result = write(output, bytes, count);
        if (result < 0)
            return false;
        count -= result;
        bytes = (uint8_t*)bytes + result;
    }
    return true;
}

void print(const char* s) {
    write_all(STDOUT_FILENO, s, strlen(s));
}

void assert(bool condition, const char* message) {
    if (condition)
        return;
    print(message);
    exit(1);
}

size_t next_alloc_size(void) {
    int category = rand() % 3;
    if (category == 0)
        return (size_t)rand() % TINY_ALLOC_MAX_SIZE;
    else if (category == 1)
        return (size_t)rand() % SMALL_ALLOC_MAX_SIZE;
    return (size_t)rand() % LARGE_ALLOC_MAX_SIZE;
}

void unoptimized_free(void* ptr) {
    free(ptr);
}

int main(void) {
    allocation_info allocations[ALLOC_COUNT];

    print("Allocating\n");
    srand(0);
    for (size_t i = 0; i < ALLOC_COUNT; ++i) {
        size_t size = next_alloc_size();
        allocations[i].size = size;

        char* result = (char*)malloc(size);
        assert(size == 0 || result != NULL, "Memory allocation failed");

        allocations[i].seed = rand();
        srand(allocations[i].seed);
        for (size_t j = 0; j < size; ++j)
            result[j] = (char)rand();

        allocations[i].data = result;
    }

    print("Reallocating\n");
    for (size_t n = 0; n < REALLOC_COUNT; ++n) {
        size_t i = rand() % ALLOC_COUNT;
        size_t new_size = next_alloc_size();
        allocations[i].data = realloc(allocations[i].data, new_size);

        size_t size = allocations[i].size;
        if (new_size < size) size = new_size;
        srand(allocations[i].seed);
        for (size_t j = 0; j < size; ++j)
            assert(allocations[i].data[j] == (char)rand(), "Corrupted\n");

        allocations[i].size = new_size;
        allocations[i].seed = rand();
        srand(allocations[i].seed);
        for (size_t j = 0; j < new_size; ++j)
            allocations[i].data[j] = (char)rand();
    }
 
    print("Checking content\n");
    for (size_t i = 0; i < ALLOC_COUNT; ++i) {
        size_t size = allocations[i].size;
        srand(allocations[i].seed);
        for (size_t j = 0; j < size; ++j)
            assert(allocations[i].data[j] == (char)rand(), "Corrupted");
    }

    print("Freeing in random order\n");
    for (size_t len = ALLOC_COUNT; len > 0;) {
        size_t i = rand() % len;
        free(allocations[i].data);
        allocations[i] = allocations[--len];
    }

    print("Everything is freed\n");
}
