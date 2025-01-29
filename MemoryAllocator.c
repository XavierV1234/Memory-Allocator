#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)     ((b) + 1)
#define BLOCK_HEADER(ptr) ((struct _block *)(ptr) - 1)

static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

// static int num_splits = 0;      // Count of block splits
// static int num_grows = 0;       // Count of heap expansions
// static int max_heap_size = 0;   // Maximum heap size reached
// static int num_coalesces = 0;   // Count of coalescing events
// static int num_requested = 0;   // Total memory requested


void printStatistics(void)
{
    printf("\nheap management statistics\n");
    printf("mallocs:\t%d\n", num_mallocs);
    printf("frees:\t\t%d\n", num_frees);
    printf("reuses:\t\t%d\n", num_reuses);
    printf("grows:\t\t%d\n", num_grows);
    printf("splits:\t\t%d\n", num_splits);
    printf("coalesces:\t%d\n", num_coalesces);
    printf("blocks:\t\t%d\n", num_blocks);
    printf("requested:\t%d\n", num_requested);
    printf("max heap:\t%d\n", max_heap);
}


struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes     */
   struct _block *next;  /* Pointer to the next _block of allocated memory      */
   struct _block *prev;  /* Pointer to the previous _block of allocated memory  */
   bool   free;          /* Is this _block free?                                */
   char   padding[3];    /* Padding: IENTRTMzMjAgU3jMDEED                       */
};

struct _block *heapList = NULL; /* Head of the free list */




 
struct _block *theLast = NULL; /* Last allocated block for Next Fit */

/* Find Free Block */
struct _block *findFreeBlock(struct _block **last, size_t size)
{
    struct _block *curr = heapList;

#if defined FIT && FIT == 0
    /* First Fit */
    while (curr && !(curr->free && curr->size >= size)) {
        *last = curr;
        curr = curr->next;
    }
#endif

#if defined BEST && BEST == 0
    /* Best Fit */
    struct _block *best = NULL;
    size_t smallest = SIZE_MAX;
    while (curr) {
        if (curr->free && curr->size >= size && curr->size < smallest) {
            best = curr;
            smallest = curr->size;
        }
        *last = curr;
        curr = curr->next;
    }
    curr = best;
   
#endif

#if defined WORST && WORST == 0
    struct _block *worst = NULL;
    size_t largest = 0;
    while (curr) {
        if (curr->free && curr->size >= size && curr->size > largest) {
            worst = curr;
            largest = curr->size;
        }
        *last = curr;
        curr = curr->next;
    }
    curr = worst;
#endif


#if defined NEXT && NEXT == 0
    curr = theLast;
    while (curr && !(curr->free && curr->size >= size)) {
        *last = curr;
        curr = curr->next;
    }
    if (!curr) {
        curr = heapList;
        while (curr && !(curr->free && curr->size >= size)) {
            *last = curr;
            curr = curr->next;
        }
    }
    theLast = curr;
#endif

    return curr;
}


struct _block *growHeap(struct _block *last, size_t size)
{
    struct _block *curr = (struct _block *)sbrk(0);
    struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

    assert(curr == prev);

    if (curr == (struct _block *)-1) {
        return NULL; /* OS allocation failed */
    }

    if (!heapList) {
        heapList = curr; /* Set heapList if not set */
    }

    if (last) {
        last->next = curr; /* Attach new block */
    }

    curr->size = size;
    curr->next = NULL;
    curr->free = false;
   
    max_heap += size;
    num_grows++;


    num_blocks++;

    return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size)
{
    if (atexit_registered == 0) {
        atexit_registered = 1;
        atexit(printStatistics);
    }

    size = ALIGN4(size);

    if (size == 0) {
        return NULL;
    }


    struct _block *last = heapList;
    struct _block *next = findFreeBlock(&last, size);
   

    if (next && (next->size - size) > sizeof(struct _block) + 4) 
    {
        struct _block *new_block = (struct _block *)((char *)BLOCK_DATA(next) + size);
        new_block->size = next->size - size - sizeof(struct _block);
        new_block->free = true;
        new_block->next = next->next;

        next->size = size;
        next->next = new_block;

        num_splits++;
    }

    if (!next) 
    {
        next = growHeap(last, size);
    } else {
        num_reuses++;
    }

    if (!next) 
    {
        return NULL;
    }

    next->free = false;
    num_requested += size;
    num_mallocs++;

   /* Return data address associated with _block to the user */
    return BLOCK_DATA(next);
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr)
{
    if (!ptr) 
    {
        return;
    }

    struct _block *curr = BLOCK_HEADER(ptr);
    assert(!curr->free);

    curr->free = true;

    curr = heapList;

    while (curr) 
    {
        if (curr->next && curr->free && curr->next->free) 
        {
            curr->size += curr->next->size + sizeof(struct _block);
            curr->next = curr->next->next;
            num_coalesces++;
        }
        curr = curr->next;
    }

    num_frees++;
}


void *calloc(size_t nmemb, size_t size)
{
    size_t total_size = nmemb * size;
    void *ptr = malloc(total_size);

    if (ptr) 
    {
        memset(ptr, 0, total_size);
    }

    return ptr;
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr) 
    {
        return malloc(size);
    }
    if (size == 0) 
    {
        free(ptr);
        return NULL;
    }

    struct _block *curr = BLOCK_HEADER(ptr);
    if (curr->size >= size) 
    {
        return ptr;
    }

    void *new_ptr = malloc(size);
    if (new_ptr) 
    {
        memcpy(new_ptr, ptr, curr->size);
        free(ptr);
    }

    return new_ptr;
}

