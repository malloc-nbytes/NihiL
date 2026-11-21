#ifndef GRAMMAR_H_INCLUDED
#define GRAMMAR_H_INCLUDED

#include "lexer.h"
#include "location.h"
#include "types.h"
#include "array.h"

#define PROC_FLAG_EXPORT   (1 << 0)
#define PROC_FLAG_EXTERN   (1 << 1)
#define PROC_FLAG_VARIADIC (1 << 2)

typedef struct visitor visitor;

typedef enum {
        EXPR_KIND_INTEGER_LITERAL = 0,
        EXPR_KIND_STRING_LITERAL,
        EXPR_KIND_IDENTIFIER,
        EXPR_KIND_BINARY,
} expr_kind;

typedef enum {
        STMT_KIND_EXPR = 0,
        STMT_KIND_LET,
        STMT_KIND_BLOCK,
        STMT_KIND_PROC,
} stmt_kind;

typedef struct expr {
        expr_kind k;
        location loc;
        void *(*accept)(struct expr *, visitor *);
} expr;

typedef struct {
        expr base;
        const token *i;
} expr_integer;

typedef struct {
        expr base;
        const token *s;
} expr_string;

typedef struct {
        expr base;
        const token *id;
} expr_identifier;

typedef struct {
        expr base;
        expr *lhs;
        const token *op;
        expr *rhs;
} expr_binary;

typedef struct stmt {
        stmt_kind k;
        location loc;
        void *(*accept)(struct stmt *, visitor *);
} stmt;

ARRAY_DEFINE(stmt *, stmtp_ar);

typedef struct {
        stmt base;
        expr *e;
} stmt_expr;

typedef struct {
        stmt base;
        const token *id;
        type *ty;
        expr *e; // optional
} stmt_let;

typedef struct {
        stmt base;
        stmtp_ar stmts;
} stmt_block;

typedef struct {
        const token *id;
        const type *ty;
} proc_param;

ARRAY_DEFINE(proc_param, proc_param_ar);

typedef struct {
        stmt base;
        uint32_t flags;
        const token *id;
        proc_param_ar params;
        type *ty;
        stmt_block *body; // NULL for extern
} stmt_proc;

expr_integer *expr_integer_alloc(const token *i, arena *a);
expr_string *expr_string_alloc(const token *s, arena *a);
expr_identifier *expr_identifier_alloc(const token *id, arena *a);
expr_binary *expr_binary_alloc(expr *lhs, const token *op, expr *rhs, arena *a);

stmt_expr *stmt_expr_alloc(expr *e, arena *a);
stmt_let *stmt_let_alloc(const token *id, type *ty, expr *e, location loc, arena *a);
stmt_block *stmt_block_alloc(stmtp_ar stmts, location loc, arena *a);
stmt_proc *stmt_proc_alloc(uint32_t flags, const token *id, proc_param_ar params, type *ty, stmt_block *body, location loc, arena *a);

#endif // GRAMMAR_H_INCLUDED
