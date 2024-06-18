#include <stdlib.h>

int main(void) {
    for (size_t i = 0; i < 1024; ++i)
        malloc(1024);
}
