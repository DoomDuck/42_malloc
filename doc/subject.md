# Subject

## Required functionalities
- Host dependent library 
- Correctly alligned allocations

## Required functions
- malloc
- realloc
- free
- show_alloc_mem

### show_alloc_mem

> The visual will be formatted by increasing addresses

```text
A rather UNIX project malloc
• Each zone must contain at least 100 allocations.
◦ “TINY” mallocs, from 1 to n bytes, will be stored in N bytes big zones.
◦ “SMALL” mallocs, from (n+1) to m bytes, will be stored in M bytes big zones.
◦ “LARGE” mallocs, fron (m+1) bytes and more, will be stored out of zone,
which simply means with mmap(), they will be in a zone on their own.
• It’s up to you to define the size of n, m, N and M so that you find a good compromise
between speed (saving on system recall) and saving memory.
You also must write a function that allows visual on the state of the allocated memory
zones. It needs to be prototyped as follows:
void show_alloc_mem();
The visual will be formatted by increasing addresses such as:
TINY : 0xA0000
0xA0020 - 0xA004A : 42 bytes
0xA006A - 0xA00BE : 84 bytes
SMALL : 0xAD000
0xAD020 - 0xADEAD : 3725 bytes
LARGE : 0xB0000
0xB0020 - 0xBBEEF : 48847 bytes
Total : 52698 bytes
```

## Authorized functions
- mmap
- munmap
- getpagesize
- getrlimit

## Architectural requirements

### Constants
Theses are values that must be *adjustable* and *tuned*

#### ZONE sizes
- MAX_TINY_ZONE_SIZE
- MAX_SMALL_ZONE_SIZE

#### ALLOCATION sizes
- MAX_TINY_ALLOCATION_SIZE
- MAX_SMALL_ALLOCATION_SIZE
- MAX_LARGE_ALLOCATION_SIZE

### 3 Memory "zones"
- Tiny
- Small
- Large

# Bonus

## Additionnal functions
- show_alloc_mem_ex

### show_alloc_mem_ex
Just add in more details :shrug:

## Additionnal functionalities
- Thread safety
- Memory defragmentation
- Environment based debugging

## Authorized functions
- all off libpthread
