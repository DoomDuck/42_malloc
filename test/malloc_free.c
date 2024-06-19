#include <stdlib.h>

int main(void) {
    int* ptr = malloc(1024);
    free(ptr);
}
