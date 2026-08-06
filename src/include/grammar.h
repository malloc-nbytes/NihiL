#ifndef GRAMMAR_H_INCLUDED
#define GRAMMAR_H_INCLUDED

typedef enum {
        EXPR_KIND_INT = 0,
        EXPR_KIND_BINARY,
} expr_kind;

typedef enum {
        STMT_KIND_LET = 0,
} stmt_kind;

typedef struct {
        expr_kind k;
        location loc;
} expr;

typedef struct {
        stmt_kind k;
        location loc;
} stmt;

typedef struct {
        
} expr_int;

#endif // GRAMMAR_H_INCLUDED
