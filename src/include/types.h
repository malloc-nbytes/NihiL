#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#define TYPE_I32   "i32"
#define TYPE_U8    "u8"
#define TYPE_VOID  "void"

#define TYPES_CPL { \
        TYPE_I32,   \
        TYPE_U8,    \
        TYPE_VOID,  \
}

int is_type(const char *s);

#endif // TYPES_H_INCLUDED
