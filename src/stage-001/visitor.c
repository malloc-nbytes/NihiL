#include "visitor.h"

visitor
visitor_create(void                          *context,
               visit_expr_integer_sig         visit_expr_integer,
               visit_expr_string_sig          visit_expr_string,
               visit_expr_identifier_sig      visit_expr_identifier,
               visit_expr_binary_sig          visit_expr_binary,
               visit_expr_call_sig            visit_expr_call,
               visit_expr_struct_literal_sig  visit_expr_struct_literal,
               visit_expr_member_sig          visit_expr_member,
               visit_expr_deref_sig           visit_expr_deref,
               visit_expr_address_sig         visit_expr_address,
               visit_expr_array_literal_sig   visit_expr_array_literal,
               visit_expr_index_sig           visit_expr_index,
               visit_expr_cast_sig            visit_expr_cast,

               visit_stmt_expr_sig            visit_stmt_expr,
               visit_stmt_let_sig             visit_stmt_let,
               visit_stmt_block_sig           visit_stmt_block,
               visit_stmt_proc_sig            visit_stmt_proc,
               visit_stmt_if_sig              visit_stmt_if,
               visit_stmt_while_sig           visit_stmt_while,
               visit_stmt_return_sig          visit_stmt_return,
               visit_stmt_struct_sig          visit_stmt_struct,
               visit_stmt_exit_sig            visit_stmt_exit)
{
        return (visitor) {
                .context = context,
                .visit_expr_integer = visit_expr_integer,
                .visit_expr_string = visit_expr_string,
                .visit_expr_identifier = visit_expr_identifier,
                .visit_expr_binary = visit_expr_binary,
                .visit_expr_call = visit_expr_call,
                .visit_expr_struct_literal = visit_expr_struct_literal,
                .visit_expr_member = visit_expr_member,
                .visit_expr_deref = visit_expr_deref,
                .visit_expr_address = visit_expr_address,
                .visit_expr_array_literal = visit_expr_array_literal,
                .visit_expr_index = visit_expr_index,
                .visit_expr_cast = visit_expr_cast,

                .visit_stmt_expr = visit_stmt_expr,
                .visit_stmt_let = visit_stmt_let,
                .visit_stmt_block = visit_stmt_block,
                .visit_stmt_proc = visit_stmt_proc,
                .visit_stmt_if = visit_stmt_if,
                .visit_stmt_while = visit_stmt_while,
                .visit_stmt_return = visit_stmt_return,
                .visit_stmt_struct = visit_stmt_struct,
                .visit_stmt_exit = visit_stmt_exit,
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
accept_expr_call(expr *e, visitor *v)
{
        if (v->visit_expr_call)
                return v->visit_expr_call(v, (expr_call *)e);
        return NULL;
}

void *
accept_expr_struct_literal(expr *e, visitor *v)
{
        if (v->visit_expr_struct_literal)
                return v->visit_expr_struct_literal(v, (expr_struct_literal *)e);
        return NULL;
}

void *
accept_expr_member(expr *e, visitor *v)
{
        if (v->visit_expr_member)
                return v->visit_expr_member(v, (expr_member *)e);
        return NULL;
}

void *
accept_expr_deref(expr *e, visitor *v)
{
        if (v->visit_expr_deref)
                return v->visit_expr_deref(v, (expr_deref *)e);
        return NULL;
}

void *
accept_expr_address(expr *e, visitor *v)
{
        if (v->visit_expr_address)
                return v->visit_expr_address(v, (expr_address *)e);
        return NULL;
}

void *
accept_expr_array_literal(expr *e, visitor *v)
{
        if (v->visit_expr_array_literal)
                return v->visit_expr_array_literal(v, (expr_array_literal *)e);
        return NULL;
}

void *
accept_expr_index(expr *e, visitor *v)
{
        if (v->visit_expr_index)
                return v->visit_expr_index(v, (expr_index *)e);
        return NULL;
}

void *
accept_expr_cast(expr *e, visitor *v)
{
        if (v->visit_expr_cast)
                return v->visit_expr_cast(v, (expr_cast *)e);
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

void *
accept_stmt_if(stmt *s, visitor *v)
{
        if (v->visit_stmt_if)
                return v->visit_stmt_if(v, (stmt_if *)s);
        return NULL;
}

void *
accept_stmt_while(stmt *s, visitor *v)
{
        if (v->visit_stmt_while)
                return v->visit_stmt_while(v, (stmt_while *)s);
        return NULL;
}

void *
accept_stmt_return(stmt *s, visitor *v)
{
        if (v->visit_stmt_return)
                return v->visit_stmt_return(v, (stmt_return *)s);
        return NULL;
}

void *
accept_stmt_struct(stmt *s, visitor *v)
{
        if (v->visit_stmt_struct)
                return v->visit_stmt_struct(v, (stmt_struct *)s);
        return NULL;
}

void *
accept_stmt_exit(stmt *s, visitor *v)
{
        if (v->visit_stmt_exit)
                return v->visit_stmt_exit(v, (stmt_exit *)s);
        return NULL;
}
