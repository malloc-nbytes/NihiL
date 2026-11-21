#ifndef ARENA_H_INCLUDED
#define ARENA_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#define ARENA_DEFAULT_BLK_SIZE 4096

typedef struct arena {
        uint8_t *data;
        size_t block_size;
        size_t capacity;
        size_t used;
        struct arena *next;
} arena;

void arena_init(arena *a, size_t block_size);
uint8_t *arena_alloc(arena *a, size_t size);
void arena_reset(arena *a);
void arena_destroy(arena *a);

#endif
