CC = cc
CPPFLAGS = -Iinclude
CFLAGS = -std=c17 -O2 -Wall -Wextra -Wpedantic -Werror

.PHONY: all test clean
all: build/demo

build:
	mkdir -p build

build/demo: src/main.c src/memory_pool.c include/memory_pool.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) src/main.c src/memory_pool.c -o $@

build/tests: tests/memory_pool_test.c src/memory_pool.c include/memory_pool.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/memory_pool_test.c src/memory_pool.c -o $@

test: build/tests
	./build/tests

clean:
	rm -rf build
