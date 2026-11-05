#include "memory_pool.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL at line %d: %s\n", __LINE__, #condition); \
        return EXIT_FAILURE; \
    } \
} while (0)

int main(void)
{
    MemoryPool pool, other;
    pool_init(&pool);
    pool_init(&other);
    void *pointers[POOL_BLOCK_COUNT];
    CHECK(pool.used == 0 && pool.peak == 0 && pool_available(&pool) == 64);
    for (size_t i = 0; i < POOL_BLOCK_COUNT; ++i) {
        pointers[i] = pool_allocate(&pool);
        CHECK(pointers[i] != NULL && pool.occupied[i]);
        CHECK((uintptr_t)pointers[i] % _Alignof(max_align_t) == 0);
        for (size_t j = 0; j < i; ++j) {
            CHECK(pointers[i] != pointers[j]);
        }
        memset(pointers[i], (int)i, POOL_BLOCK_SIZE);
    }
    CHECK(pool_allocate(&pool) == NULL);
    CHECK(pool.used == 64 && pool.peak == 64 && pool_available(&pool) == 0);
    for (size_t i = 0; i < POOL_BLOCK_COUNT; ++i) {
        const unsigned char *bytes = pointers[i];
        for (size_t j = 0; j < POOL_BLOCK_SIZE; ++j) {
            CHECK(bytes[j] == (unsigned char)i);
        }
    }
    int foreign = 0;
    void *other_pointer = pool_allocate(&other);
    CHECK(pool_deallocate(&pool, &foreign) == POOL_INVALID_POINTER);
    CHECK(pool_deallocate(&pool, other_pointer) == POOL_INVALID_POINTER);
    CHECK(pool_deallocate(&pool, (unsigned char *)pointers[0] + 1)
          == POOL_INVALID_POINTER);
    CHECK(pool_deallocate(&pool, NULL) == POOL_OK);
    CHECK(pool.used == 64 && pool.peak == 64);
    CHECK(pool_deallocate(&pool, pointers[17]) == POOL_OK);
    CHECK(!pool.occupied[17] && pool.used == 63);
    CHECK(pool_deallocate(&pool, pointers[17]) == POOL_DOUBLE_RELEASE);
    CHECK(pool.used == 63);
    CHECK(pool_allocate(&pool) == pointers[17]);
    for (size_t i = 0; i < POOL_BLOCK_COUNT; ++i) {
        CHECK(pool_deallocate(&pool, pointers[i]) == POOL_OK);
    }
    CHECK(pool.used == 0 && pool.peak == 64 && pool_available(&pool) == 64);
    for (size_t round = 0; round < 100; ++round) {
        for (size_t i = 0; i < POOL_BLOCK_COUNT; ++i) {
            pointers[i] = pool_allocate(&pool);
            CHECK(pointers[i] != NULL);
        }
        CHECK(pool_allocate(&pool) == NULL);
        for (size_t i = POOL_BLOCK_COUNT; i > 0; --i) {
            CHECK(pool_deallocate(&pool, pointers[i - 1]) == POOL_OK);
        }
    }
    CHECK(pool_deallocate(&other, other_pointer) == POOL_OK);
    puts("PASS: statistics, alignment, unique blocks, payload integrity, exhaustion, "
         "invalid pointers, double release, reuse and 100 cycles");
    return EXIT_SUCCESS;
}
