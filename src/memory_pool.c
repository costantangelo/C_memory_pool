#include "memory_pool.h"

_Static_assert(POOL_BLOCK_SIZE % _Alignof(max_align_t) == 0,
               "Block size must be a multiple of the alignment");

void pool_init(MemoryPool *pool)
{
    for (size_t i = 0; i < POOL_BLOCK_COUNT; ++i) {
        pool->next[i] = i + 1;
        pool->occupied[i] = false;
    }
    pool->head = 0;
    pool->used = 0;
    pool->peak = 0;
}

void *pool_allocate(MemoryPool *pool)
{
    if (pool->head == POOL_BLOCK_COUNT) {
        return NULL;
    }
    size_t index = pool->head;
    pool->head = pool->next[index];
    pool->occupied[index] = true;
    ++pool->used;
    if (pool->used > pool->peak) {
        pool->peak = pool->used;
    }
    return pool->blocks[index].bytes;
}

PoolResult pool_deallocate(MemoryPool *pool, void *pointer)
{
    if (pointer == NULL) {
        return POOL_OK;
    }
    /* Compare addresses without subtracting unrelated pointers. */
    for (size_t i = 0; i < POOL_BLOCK_COUNT; ++i) {
        if (pointer == (void *)pool->blocks[i].bytes) {
            if (!pool->occupied[i]) {
                return POOL_DOUBLE_RELEASE;
            }
            pool->occupied[i] = false;
            pool->next[i] = pool->head;
            pool->head = i;
            --pool->used;
            return POOL_OK;
        }
    }
    return POOL_INVALID_POINTER;
}

size_t pool_available(const MemoryPool *pool)
{
    return POOL_BLOCK_COUNT - pool->used;
}
