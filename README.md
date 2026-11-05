# C_memory_pool

A small fixed-block memory pool written in C17. It manages 64 blocks of 64 bytes
using an index-based free list. The project includes a terminal demo and tests.
It does not call `malloc` or require external libraries.

## Build and run

Requirements: a C17 compiler and GNU Make. From the repository directory:

```sh
make
./build/demo
make test
```

The demo displays free (`[.]`) and occupied (`[X]`) blocks as it allocates eight
blocks, releases and reuses block 3, fills the pool, and releases everything.
Each row contains 16 blocks, numbered 0–63 in row order. It also demonstrates
storing and recovering an integer using `memcpy` and detecting a double release.

## API

```c
MemoryPool pool;
pool_init(&pool);
void *block = pool_allocate(&pool);
if (block != NULL) {
    /* Store up to POOL_BLOCK_SIZE bytes. */
    PoolResult result = pool_deallocate(&pool, block);
    /* Check result == POOL_OK. */
    (void)result;
}
```

- `pool_init`: initialize an unused pool before its first allocation.
- `pool_allocate`: return a block, or `NULL` when full. O(1) time.
- `pool_deallocate`: return `POOL_OK`, `POOL_INVALID_POINTER`, or
  `POOL_DOUBLE_RELEASE`. Releasing `NULL` is a successful no-op.
- `pool_available`: report the number of free blocks.
- `used`, `peak`, and `occupied` expose statistics for read-only inspection.

The free list stores indices in `next`; `head` identifies its first block.
Index 64 marks the end of the list. Allocation removes the head. Release checks
the pointer and inserts the block at the head. Pointer validation scans all
64 block starts, so release takes O(N), with N=64. Invalid releases preserve state.

## Memory model and limitations

- Usable capacity is 4096 bytes; metadata adds overhead to `sizeof(MemoryPool)`.
- Every request occupies 64 bytes, even when fewer bytes are needed. This is
  internal fragmentation. Blocks cannot be combined for larger requests.
- Each block is aligned to `_Alignof(max_align_t)`. Over-aligned types are unsupported.
- Storage is declared as byte arrays. For portable C17, use `memcpy` to store and
  retrieve typed values through local objects; do not dereference a cast to an
  unrelated typed pointer into these arrays. Alignment alone does not remove
  C's effective-type restrictions. This is a byte-storage pool, not a full
  replacement for `malloc`.
- Pool arguments must be non-null and initialized. Do not modify metadata.
- Never copy or move an initialized pool, or reinitialize it while blocks are
  in use. C does not enforce these restrictions.
- The pool must outlive all its allocations. Never access a block after release.
- Released storage is not cleared. A stale pointer to a block that has since
  been reallocated cannot be distinguished from its new owner's pointer.
- Single-threaded only; no synchronization or automatic resource cleanup.

Tests cover exhaustion, alignment, unique blocks, payload integrity, invalid and
interior pointers, double release, statistics, and 100 full allocation/release cycles.
This is a learning project. It has not been validated for safety-critical use.
