#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <stdbool.h>
#include <stddef.h>

enum { POOL_BLOCK_SIZE = 64, POOL_BLOCK_COUNT = 64 };

typedef enum {
    POOL_OK,
    POOL_INVALID_POINTER,
    POOL_DOUBLE_RELEASE
} PoolResult;

typedef struct {
    _Alignas(max_align_t) unsigned char bytes[POOL_BLOCK_SIZE];
} PoolBlock;

typedef struct {
    PoolBlock blocks[POOL_BLOCK_COUNT];
    size_t next[POOL_BLOCK_COUNT];
    bool occupied[POOL_BLOCK_COUNT];
    size_t head;
    size_t used;
    size_t peak;
} MemoryPool;

/* Initialize once before use. Never copy or move an initialized pool. */
void pool_init(MemoryPool *pool);
/* Returns NULL when full. Allocations contain unspecified bytes. */
void *pool_allocate(MemoryPool *pool);
/* Releasing NULL is a successful no-op. Invalid releases preserve state. */
PoolResult pool_deallocate(MemoryPool *pool, void *pointer);
size_t pool_available(const MemoryPool *pool);

#endif
