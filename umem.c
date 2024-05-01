#include "umem.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif

#define MAX_ALLOC_SIZE (1024 * 1024 * 10)

typedef struct block
{
    size_t size;
    struct block *next;
    int free;
} block_t;

#define BLOCK_SIZE sizeof(block_t)

void split_block(block_t *b, size_t size);
void *base = NULL;
block_t *freeList = NULL;
int alloc_algo = FIRST_FIT;
block_t *lastAllocated = NULL;

block_t *find_block(size_t size)
{
    block_t *current = freeList;
    block_t *best_fit = NULL;
    block_t *worst_fit = NULL;

    while (current)
    {
        if (current->free && current->size >= size)
        {
            switch (alloc_algo)
            {
            case FIRST_FIT:
                return current;
            case BEST_FIT:
                if (!best_fit || current->size < best_fit->size)
                {
                    best_fit = current;
                }
                break;
            case WORST_FIT:
                if (!worst_fit || current->size > worst_fit->size)
                {
                    worst_fit = current;
                }
                break;
            case NEXT_FIT:
                if (lastAllocated && current < lastAllocated)
                {
                    current = current->next;
                    continue;
                }
                lastAllocated = current;
                return current;
            }
        }
        current = current->next;
    }
    return (alloc_algo == BEST_FIT) ? best_fit : ((alloc_algo == WORST_FIT) ? worst_fit : NULL);
}

block_t *extend_heap(block_t *last, size_t size)
{
    block_t *block;
    block = mmap(0, size + BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (block == MAP_FAILED)
    {
        return NULL;
    }
    block->size = size;
    block->free = 0;
    block->next = NULL;

    if (last)
    {
        last->next = block;
    }
    return block;
}

void *umalloc(size_t size)
{
    if (size == 0 || size > MAX_ALLOC_SIZE)
    {
        return NULL;
    }
    size = (size + 7) & ~7;

    block_t *block;
    if (!freeList)
    {
        block = extend_heap(NULL, size);
        if (!block)
        {
            return NULL;
        }
        freeList = block;
    }
    else
    {
        block = find_block(size);
        if (!block)
        {
            block = extend_heap(freeList, size);
            if (!block)
            {
                return NULL;
            }
        }
        else
        {
            split_block(block, size);
        }
    }

    block->free = 0;

    uintptr_t start_addr = (uintptr_t)(block + 1);
    uintptr_t misalignment = (uintptr_t)(start_addr % 8);
    if (misalignment != 0)
    {
        start_addr += (8 - misalignment);
    }

    block->size -= (start_addr - (uintptr_t)(block + 1));

    return (void *)start_addr;
}

int ufree(void *ptr)
{
    if (!ptr)
    {
        return -1;
    }
    block_t *block = (block_t *)ptr - 1;
    block->free = 1;
    return 0;
}

void umemdump()
{
    block_t *current = freeList;
    while (current)
    {
        printf("Block: %p, size: %zu, free: %d\n", (void *)current, current->size, current->free);
        current = current->next;
    }
}

int umeminit(size_t sizeOfRegion, int allocationAlgo)
{
    if (base)
    {
        return -1;
    }
    base = mmap(0, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED)
    {
        return -1;
    }
    freeList = (block_t *)base;
    freeList->size = sizeOfRegion - BLOCK_SIZE;
    freeList->free = 1;
    freeList->next = NULL;
    alloc_algo = allocationAlgo;
    return 0;
}

void split_block(block_t *b, size_t size)
{
    if (b->size > size + BLOCK_SIZE + 1)
    {
        block_t *new_block = (block_t *)((char *)b + size + BLOCK_SIZE);
        new_block->size = b->size - size - BLOCK_SIZE;
        new_block->free = 1;
        new_block->next = b->next;
        b->next = new_block;
    }
    b->size = size;
    b->free = 0;
}
