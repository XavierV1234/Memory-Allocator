# Memory-Allocator
## The Summary of Memory-Allocator
This C program implements a custom memory allocator that provides alternatives to standard malloc, free, calloc, and realloc functions. The implementation manages dynamic memory allocation within a simulated heap using linked list-based block management. The program also tracks and reports various memory statistics, such as the number of allocations, deallocations, memory reuse, heap expansions, and block merges.

## Key Features of Memory-Allocator

### Custom Memory Allocation (malloc)

-Allocates memory dynamically while keeping track of memory blocks.

-Searches for a free block using different allocation strategies (First Fit, Best Fit, Worst Fit, Next Fit).

-Expands the heap if no suitable block is found.

-Supports block splitting when a large free block is available.

### Custom Deallocation (free)

-Marks a memory block as free.

-Merges adjacent free blocks (coalescing) to prevent fragmentation.

### Heap Expansion (growHeap)

-Uses sbrk() to request additional memory from the operating system.

-Maintains a linked list of allocated and free memory blocks.

### Zero-Initialized Allocation (calloc)

-Allocates memory and initializes it to zero.
### Memory Reallocation (realloc)

-Expands or shrinks an existing memory allocation.
-If a larger block is needed, a new block is allocated, and old data is copied.

### Memory Statistics (printStatistics)

Tracks and displays key metrics like:

-Number of malloc/free calls

-Heap expansions

-Memory reuse

-Block splits and merges

-Total memory requested

-Peak heap usage
