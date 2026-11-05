#include "memory_pool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void show(const MemoryPool *pool, const char *stage)
{
    printf("\n%s\nUsed: %zu | Available: %zu | Peak: %zu\n",
           stage, pool->used, pool_available(pool), pool->peak);
    for (size_t i = 0; i < POOL_BLOCK_COUNT; ++i) {
        printf("[%c]%s", pool->occupied[i] ? 'X' : '.',
               (i + 1) % 16 == 0 ? "\n" : "");
    }
}

int main(void)
{
    MemoryPool pool;
    pool_init(&pool);
    void *pointers[POOL_BLOCK_COUNT] = {0};
    puts("C17 memory pool: 64 blocks x 64 bytes\n[.] free | [X] occupied");
    show(&pool, "1. Empty pool");
    for (size_t i = 0; i < 8; ++i) {
        pointers[i] = pool_allocate(&pool);
        if (pointers[i] == NULL) {
            return EXIT_FAILURE;
        }
    }
    /* Copy object representations into byte storage; avoid typed pointer casts. */
    int value = 42;
    int recovered = 0;
    memcpy(pointers[0], &value, sizeof value);
    memcpy(&recovered, pointers[0], sizeof recovered);
    printf("Stored and recovered integer: %d\n", recovered);
    show(&pool, "2. Eight allocations");
    void *released = pointers[3];
    if (pool_deallocate(&pool, released) != POOL_OK) {
        return EXIT_FAILURE;
    }
    show(&pool, "3. Block 3 released");
    if (pool_deallocate(&pool, released) != POOL_DOUBLE_RELEASE) {
        return EXIT_FAILURE;
    }
    puts("Double release detected.");
    pointers[3] = pool_allocate(&pool);
    if (pointers[3] != released) {
        return EXIT_FAILURE;
    }
    show(&pool, "4. Block 3 reused");
    for (size_t i = 8; i < POOL_BLOCK_COUNT; ++i) {
        pointers[i] = pool_allocate(&pool);
        if (pointers[i] == NULL) {
            return EXIT_FAILURE;
        }
    }
    show(&pool, "5. Full pool");
    if (pool_allocate(&pool) != NULL) {
        return EXIT_FAILURE;
    }
    puts("Pool exhausted: allocation returns NULL.");
    for (size_t i = 0; i < POOL_BLOCK_COUNT; ++i) {
        if (pool_deallocate(&pool, pointers[i]) != POOL_OK) {
            return EXIT_FAILURE;
        }
    }
    show(&pool, "6. All blocks released");
    return recovered == value ? EXIT_SUCCESS : EXIT_FAILURE;
}
