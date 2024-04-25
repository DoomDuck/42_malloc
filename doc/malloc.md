# Malloc

## Lexicon

### Arena
> A structure that is shared among one or more threads which contains references to one or more heaps, as well as linked lists of chunks within those heaps which are "free". Threads assigned to each arena will allocate memory from that arena's free lists. 

### Heap
> A contiguous region of memory that is subdivided into chunks to be allocated. Each heap belongs to exactly one arena. 

### Chunk
> A small range of memory that can be allocated (owned by the application), freed (owned by glibc), or combined with adjacent chunks into larger ranges. Note that a chunk is a wrapper around the block of memory that is given to the application. Each chunk exists in one heap and belongs to one arena. 

### Memory
> A portion of the application's address space which is typically backed by RAM or swap. 

## Sources
- <https://sourceware.org/glibc/wiki/MallocInternals>
