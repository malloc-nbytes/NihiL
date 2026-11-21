#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

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

int is_type(const char *s);

#endif // TYPES_H_INCLUDED
