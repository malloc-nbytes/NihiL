#include "visitor.h"

visitor
visitor_create(void                      *context,
               visit_expr_integer_sig     visit_expr_integer,
               visit_expr_string_sig      visit_expr_string,
               visit_expr_identifier_sig  visit_expr_identifier,
               visit_expr_binary_sig      visit_expr_binary,
               visit_stmt_expr_sig        visit_stmt_expr,
               visit_stmt_let_sig         visit_stmt_let,
               visit_stmt_block_sig       visit_stmt_block,
               visit_stmt_proc_sig        visit_stmt_proc)
{
        return (visitor) {
                .context = context,
                .visit_expr_integer = visit_expr_integer,
                .visit_expr_string = visit_expr_string,
                .visit_expr_identifier = visit_expr_identifier,
                .visit_expr_binary = visit_expr_binary,
                .visit_stmt_expr = visit_stmt_expr,
                .visit_stmt_let = visit_stmt_let,
                .visit_stmt_block = visit_stmt_block,
                .visit_stmt_proc = visit_stmt_proc,
        };
}

void *
accept_expr_integer(expr *e, visitor *v)
{
        if (v->visit_expr_integer)
                return v->visit_expr_integer(v, (expr_integer *)e);
        return NULL;
}

void *
accept_expr_string(expr *e, visitor *v)
{
        if (v->visit_expr_string)
                return v->visit_expr_string(v, (expr_string *)e);
        return NULL;
}

void *
accept_expr_identifier(expr *e, visitor *v)
{
        if (v->visit_expr_identifier)
                return v->visit_expr_identifier(v, (expr_identifier *)e);
        return NULL;
}

void *
accept_expr_binary(expr *e, visitor *v)
{
        if (v->visit_expr_binary)
                return v->visit_expr_binary(v, (expr_binary *)e);
        return NULL;
}

void *
accept_stmt_expr(stmt *s, visitor *v)
{
        if (v->visit_stmt_expr)
                return v->visit_stmt_expr(v, (stmt_expr *)s);
        return NULL;
}

void *
accept_stmt_let(stmt *s, visitor *v)
{
        if (v->visit_stmt_let)
                return v->visit_stmt_let(v, (stmt_let *)s);
        return NULL;
}

void *
accept_stmt_block(stmt *s, visitor *v)
{
        if (v->visit_stmt_block)
                return v->visit_stmt_block(v, (stmt_block *)s);
        return NULL;
}

void *
accept_stmt_proc(stmt *s, visitor *v)
{
        if (v->visit_stmt_proc)
                return v->visit_stmt_proc(v, (stmt_proc *)s);
        return NULL;
}
