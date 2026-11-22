#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include "arena.h"
#include "array.h"
#include "str.h"

#include <stddef.h>

#define TYPE_VOID "void"
#define TYPE_I8   "i8"
#define TYPE_I16  "i16"
#define TYPE_I32  "i32"
#define TYPE_I64  "i64"
#define TYPE_U8   "u8"
#define TYPE_U16  "u16"
#define TYPE_U32  "u32"
#define TYPE_U64  "u64"

#define TYPES_CPL { \
        TYPE_VOID, \
        TYPE_I8, \
        TYPE_I16, \
        TYPE_I32, \
        TYPE_I64, \
        TYPE_U8, \
        TYPE_U16, \
        TYPE_U32, \
        TYPE_U64, \
        NULL, \
}

typedef enum {
        TYPE_KIND_I8 = 0,
        TYPE_KIND_I16,
        TYPE_KIND_I32,
        TYPE_KIND_I64,
        TYPE_KIND_U8,
        TYPE_KIND_U16,
        TYPE_KIND_U32,
        TYPE_KIND_U64,
        __TYPE_INTEGERS, // for comparisons

        TYPE_KIND_VOID,
        TYPE_KIND_PTR,
        TYPE_KIND_PROC,
        TYPE_KIND_STRUCT,
        TYPE_KIND_NAMED,
} type_kind;

typedef struct { type_kind k; } type;
ARRAY_DEFINE(type *, typep_ar);

typedef struct { type base; } type_void;
typedef struct { type base; } type_i8;
typedef struct { type base; } type_i16;
typedef struct { type base; } type_i32;
typedef struct { type base; } type_i64;
typedef struct { type base; } type_u8;
typedef struct { type base; } type_u16;
typedef struct { type base; } type_u32;
typedef struct { type base; } type_u64;

typedef struct {
        type base;
        type *to;
} type_ptr;

typedef struct {
        type base;
        type *ty;
        typep_ar tys;
        int variadic;
} type_proc;

typedef struct stmt_struct stmt_struct;
typedef struct {
        type base;
        stmt_struct *declaration;
} type_struct;

typedef struct token token;
typedef struct {
        type base;
        const token *id;
} type_named;

void init_types_translation_unit(void);

type_void *type_void_alloc(void);
type_i8 *type_i8_alloc(void);
type_i16 *type_i16_alloc(void);
type_i32 *type_i32_alloc(void);
type_i64 *type_i64_alloc(void);
type_u8 *type_u8_alloc(void);
type_u16 *type_u16_alloc(void);
type_u32 *type_u32_alloc(void);
type_u64 *type_u64_alloc(void);
type_ptr *type_ptr_alloc(type *to, arena *a);
type_proc *type_proc_alloc(type *ty, typep_ar tys, int variadic, arena *a);
type_struct *type_struct_alloc(stmt_struct *declaration, arena *a);
type_named *type_named_alloc(const token *id, arena *a);

int is_type(const char *s);
int type_compatible(const type *ty0, const type *ty1);

// Note: `op' is an `int' and not `token_kind' so as to
//       not cause circular dependencies. The source file
//       casts to a `token_kind'.
int type_implements_operator(const type *const ty, int op);
str type_to_str(const type *const ty);

#endif // TYPES_H_INCLUDED
