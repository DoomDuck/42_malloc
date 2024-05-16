#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/*
 * Chunk
 */
typedef struct {
	size_t size_divided_by_16 : sizeof(size_t) - 3;
	bool arena: 1;
	bool mmaped: 1;
	bool previous_in_use: 1;
} chunk_header;

typedef union {
	uint8_t payload;
	struct {
		struct s_chunk* previous;
		struct s_chunk* next;
	} list;
} __attribute__((aligned(16))) chunk_body;

typedef struct s_chunk {
	size_t previous_chunk_size;
	chunk_header header;
	chunk_body body;
} chunk;

void chunk_init_alloc(chunk* self, size_t previous_chunk_size, size_t size);
chunk* chunk_of_payload(void* payload);
size_t chunk_size(chunk* self);

/*
 * Page
 */
typedef struct s_page {
	size_t size;
	struct s_page* next;
	chunk first_chunk;
} page;

page* page_of_first_chunk(chunk* first);
