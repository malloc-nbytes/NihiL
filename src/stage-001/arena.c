#include "arena.h"

#include <stdlib.h>

static size_t
align_up(size_t value, size_t alignment)
{
        size_t remainder;

        remainder = value % alignment;
        if (remainder == 0)
                return value;

        if (value > SIZE_MAX - (alignment - remainder))
                return SIZE_MAX;

        return value + alignment - remainder;
}

static int
arena_add_block(arena *a, size_t capacity)
{
        arena *block;

        if (a->data == NULL) {
                a->data = malloc(capacity);
                if (a->data == NULL)
                        return 0;

                a->capacity = capacity;
                return 1;
        }

        block = calloc(1, sizeof(*block));
        if (block == NULL)
                return 0;

        block->data = (uint8_t *)malloc(capacity);
        if (block->data == NULL) {
                free(block);
                return 0;
        }

        block->block_size = a->block_size;
        block->capacity = capacity;
        while (a->next != NULL)
                a = a->next;
        a->next = block;
        return 1;
}

void
arena_init(arena *a, size_t block_size)
{
        if (block_size == 0)
                block_size = 1;

        *a = (arena) {
                .block_size = block_size,
        };
}

uint8_t *
arena_alloc(arena *a, size_t size)
{
        arena *block;
        size_t alignment;
        size_t offset;
        size_t capacity;

        if (a == NULL || size == 0)
                return NULL;

        alignment = _Alignof(max_align_t);
        block = a;
        for (;;) {
                if (block->data != NULL) {
                        offset = align_up(block->used, alignment);
                        if (offset != SIZE_MAX && offset <= block->capacity &&
                            size <= block->capacity - offset) {
                                block->used = offset + size;
                                return block->data + offset;
                        }
                }

                if (block->next == NULL)
                        break;
                block = block->next;
        }

        if (size > SIZE_MAX - (alignment - 1))
                return NULL;
        capacity = size + alignment - 1;
        if (capacity < a->block_size)
                capacity = a->block_size;

        if (!arena_add_block(a, capacity))
                return NULL;

        block = a;
        while (block->next != NULL)
                block = block->next;
        block->used = size;
        return block->data;
}

void
arena_reset(arena *a)
{
        for (; a != NULL; a = a->next)
                a->used = 0;
}

void
arena_destroy(arena *a)
{
        arena *block;
        arena *next;

        if (a == NULL)
                return;

        free(a->data);
        for (block = a->next; block != NULL; block = next) {
                next = block->next;
                free(block->data);
                free(block);
        }
        *a = (arena) {0};
}
