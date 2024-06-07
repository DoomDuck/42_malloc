#include <mem.h>
#include <stddef.h>
#include <stdint.h>

void memory_copy(void *destination, const void *source, size_t count) {
	uint8_t *destination_array = destination;
	const uint8_t *source_array = source;
	for (size_t i = 0; i < count; ++i)
		destination_array[i] = source_array[i];
}
