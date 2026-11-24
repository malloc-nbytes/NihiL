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
typedef struct binding binding;
typedef struct struct_member struct_member;

typedef enum {
        EXPR_KIND_INTEGER_LITERAL = 0,
        EXPR_KIND_STRING_LITERAL,
        EXPR_KIND_IDENTIFIER,
        EXPR_KIND_BINARY,
        EXPR_KIND_CALL,
        EXPR_KIND_STRUCT_LITERAL,
        EXPR_KIND_MEMBER,
        EXPR_KIND_DEREF,
        EXPR_KIND_ADDRESS,
        EXPR_KIND_ARRAY_LITERAL,
        EXPR_KIND_INDEX,
        EXPR_KIND_CAST,
        EXPR_KIND_NULL,
} expr_kind;

typedef enum {
        STMT_KIND_EXPR = 0,
        STMT_KIND_LET,
        STMT_KIND_BLOCK,
        STMT_KIND_PROC,
        STMT_KIND_IF,
        STMT_KIND_WHILE,
        STMT_KIND_RETURN,
        STMT_KIND_STRUCT,
        STMT_KIND_EXIT,
} stmt_kind;

typedef struct expr {
        expr_kind k;
        location loc;
        type *ty;
        void *(*accept)(struct expr *, visitor *);
} expr;

ARRAY_DEFINE(expr *, exprp_ar);

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
        binding *bind;
} expr_identifier;

typedef struct {
        expr base;
        expr *lhs;
        const token *op;
        expr *rhs;
} expr_binary;

typedef struct {
        expr base;
        expr *lhs;
        exprp_ar exprs;
} expr_call;

typedef struct {
        const token *id;
        expr *e;
        struct_member *member;
} struct_literal_kv;

ARRAY_DEFINE(struct_literal_kv, struct_literal_kv_ar);

typedef struct {
        expr base;
        const token *for_;
        struct_literal_kv_ar setters;
} expr_struct_literal;

typedef struct {
        expr base;
        expr *lhs;
        const token *member;
        struct_member *resolved;
} expr_member;

typedef struct {
        expr base;
        expr *rhs;
} expr_deref;

typedef struct {
        expr base;
        expr *rhs;
} expr_address;

typedef struct {
        expr base;
        exprp_ar exprs;
} expr_array_literal;

typedef struct {
        expr base;
        expr *lhs;
        expr *idx;
} expr_index;

typedef struct {
        expr base;
        type *to;
        expr *rhs;
} expr_cast;

typedef struct {
        expr base;
} expr_null;

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
        binding *bind;
} stmt_let;

typedef struct {
        stmt base;
        stmtp_ar stmts;
} stmt_block;

typedef struct {
        const token *id;
        type *ty;
        binding *bind;
} proc_param;

ARRAY_DEFINE(proc_param, proc_param_ar);

typedef struct {
        stmt base;
        uint32_t flags;
        const token *id;
        proc_param_ar params;
        type *ty;
        stmt_block *body; // NULL for extern
        binding *bind;
} stmt_proc;

typedef struct {
        stmt base;
        expr *e;
        stmt *then;
        stmt *else_; // optional
} stmt_if;

typedef struct {
        stmt base;
        expr *e;
        stmt *body;
} stmt_while;

typedef struct {
        stmt base;
        expr *e; // optional
} stmt_return;

typedef struct struct_member {
        const token *id;
        type *ty;
        binding *bind;
} struct_member;

ARRAY_DEFINE(struct_member, struct_member_ar);

typedef struct stmt_struct {
        stmt base;
        const token *id;
        struct_member_ar members;
        binding *bind;
        type_struct *ty;
} stmt_struct;

typedef struct {
        stmt base;
        expr *e;
} stmt_exit;

expr_integer *expr_integer_alloc(const token *i, arena *a);
expr_string *expr_string_alloc(const token *s, arena *a);
expr_identifier *expr_identifier_alloc(const token *id, arena *a);
expr_binary *expr_binary_alloc(expr *lhs, const token *op, expr *rhs, arena *a);
expr_call *expr_call_alloc(expr *lhs, exprp_ar exprs, arena *a);
expr_struct_literal *expr_struct_literal_alloc(const token *for_, struct_literal_kv_ar setters, arena *a);
expr_member *expr_member_alloc(expr *lhs, const token *member, arena *a);
expr_deref *expr_deref_alloc(expr *rhs, location loc, arena *a);
expr_address *expr_address_alloc(expr *rhs, location loc, arena *a);
expr_array_literal *expr_array_literal_alloc(exprp_ar exprs, location loc, arena *a);
expr_index *expr_index_alloc(expr *lhs, expr *idx, location loc, arena *a);
expr_cast *expr_cast_alloc(type *to, expr *rhs, location loc, arena *a);
expr_null *expr_null_alloc(location loc, arena *a);

stmt_expr *stmt_expr_alloc(expr *e, arena *a);
stmt_let *stmt_let_alloc(const token *id, type *ty, expr *e, location loc, arena *a);
stmt_block *stmt_block_alloc(stmtp_ar stmts, location loc, arena *a);
stmt_proc *stmt_proc_alloc(uint32_t flags, const token *id, proc_param_ar params, type *ty, stmt_block *body, location loc, arena *a);
stmt_if *stmt_if_alloc(expr *e, stmt *then, stmt *else_, location loc, arena *a);
stmt_while *stmt_while_alloc(expr *e, stmt *body, location loc, arena *a);
stmt_return *stmt_return_alloc(expr *e, location loc, arena *a);
stmt_struct *stmt_struct_alloc(const token *id, struct_member_ar members, location loc, arena *a);
stmt_exit *stmt_exit_alloc(expr *e, location loc, arena *a);

#endif // GRAMMAR_H_INCLUDED
