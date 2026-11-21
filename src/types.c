#include "types.h"

#include <stdlib.h>
#include <string.h>

static type_void *g_void = NULL;
static type_i8 *g_i8 = NULL;
static type_i16 *g_i16 = NULL;
static type_i32 *g_i32 = NULL;
static type_i64 *g_i64 = NULL;
static type_u8 *g_u8 = NULL;
static type_u16 *g_u16 = NULL;
static type_u32 *g_u32 = NULL;
static type_u64 *g_u64 = NULL;

void
init_types_translation_unit(void)
{
        g_void = (type_void *)malloc(sizeof(*g_void));
        g_void->base.k = TYPE_KIND_VOID;

        g_i8 = (type_i8 *)malloc(sizeof(*g_i8));
        g_i8->base.k = TYPE_KIND_I8;

        g_i16 = (type_i16 *)malloc(sizeof(*g_i16));
        g_i16->base.k = TYPE_KIND_I16;

        g_i32 = (type_i32 *)malloc(sizeof(*g_i32));
        g_i32->base.k = TYPE_KIND_I32;

        g_i64 = (type_i64 *)malloc(sizeof(*g_i64));
        g_i64->base.k = TYPE_KIND_I64;

        g_u8 = (type_u8 *)malloc(sizeof(*g_u8));
        g_u8->base.k = TYPE_KIND_U8;

        g_u16 = (type_u16 *)malloc(sizeof(*g_u16));
        g_u16->base.k = TYPE_KIND_U16;

        g_u32 = (type_u32 *)malloc(sizeof(*g_u32));
        g_u32->base.k = TYPE_KIND_U32;

        g_u64 = (type_u64 *)malloc(sizeof(*g_u64));
        g_u64->base.k = TYPE_KIND_U64;
}

int
is_type(const char *s)
{
        static const char *kwds[] = TYPES_CPL;

        for (size_t i = 0; kwds[i]; ++i) {
                if (!strcmp(s, kwds[i]))
                        return 1;
        }

        return 0;
}

type_void *
type_void_alloc(void)
{
        return g_void;
}

type_i8 *
type_i8_alloc(void)
{
        return g_i8;
}

type_i16 *
type_i16_alloc(void)
{
        return g_i16;
}

type_i32 *
type_i32_alloc(void)
{
        return g_i32;
}

type_i64 *
type_i64_alloc(void)
{
        return g_i64;
}

type_u8 *
type_u8_alloc(void)
{
        return g_u8;
}

type_u16 *
type_u16_alloc(void)
{
        return g_u16;
}

type_u32 *
type_u32_alloc(void)
{
        return g_u32;
}

type_u64 *
type_u64_alloc(void)
{
        return g_u64;
}

type_ptr *
type_ptr_alloc(type *to, arena *a)
{
        type_ptr *t;

        t = (type_ptr *)arena_alloc(a, sizeof(*t));
        t->to = to;
        t->base.k = TYPE_KIND_PTR;

        return t;
}

type_proc *
type_proc_alloc(type *ty, typep_ar tys, arena *a)
{
        type_proc *t;

        t = (type_proc *)arena_alloc(a, sizeof(*t));
        t->ty = ty;
        t->tys = tys;
        t->base.k = TYPE_KIND_PROC;

        return t;
}

int
type_compatible(const type *ty0,
                const type *ty1)
{
        if (ty0->k < TYPE_INTEGERS && ty1->k < TYPE_INTEGERS)
                return 1;

        if (ty0->k != ty1->k)
                return 0;

        // ty0.k == ty1.k from here

        if (ty0->k == TYPE_KIND_PTR)
                return type_compatible(((const type_ptr *)ty0)->to,
                                       ((const type_ptr *)ty0)->to);

        if (ty0->k == TYPE_KIND_PROC) {
                const type_proc *pty0 = (const type_proc *)ty0;
                const type_proc *pty1 = (const type_proc *)ty1;

                if (pty0->tys.len != pty1->tys.len)
                        return 0;

                for (size_t i = 0; i < pty0->tys.len; ++i) {
                        if (!type_compatible(pty0->tys.data[i], pty1->tys.data[i]))
                                return 0;
                }

                return type_compatible(pty0->ty, pty1->ty);
        }

        return 0;
}
