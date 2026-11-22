#ifndef VISITOR_H_INCLUDED
#define VISITOR_H_INCLUDED

#include "grammar.h"

typedef void *(*visit_expr_integer_sig)(visitor *, expr_integer *);
typedef void *(*visit_expr_string_sig)(visitor *, expr_string *);
typedef void *(*visit_expr_identifier_sig)(visitor *, expr_identifier *);
typedef void *(*visit_expr_binary_sig)(visitor *, expr_binary *);
typedef void *(*visit_expr_call_sig)(visitor *, expr_call *);

typedef void *(*visit_stmt_expr_sig)(visitor *, stmt_expr *);
typedef void *(*visit_stmt_let_sig)(visitor *, stmt_let *);
typedef void *(*visit_stmt_block_sig)(visitor *, stmt_block *);
typedef void *(*visit_stmt_proc_sig)(visitor *, stmt_proc *);
typedef void *(*visit_stmt_if_sig)(visitor *, stmt_if *);
typedef void *(*visit_stmt_while_sig)(visitor *, stmt_while *);
typedef void *(*visit_stmt_return_sig)(visitor *, stmt_return *);
typedef void *(*visit_stmt_struct_sig)(visitor *, stmt_struct *);
typedef void *(*visit_stmt_exit_sig)(visitor *, stmt_exit *);

typedef struct visitor {
        void *context;

        visit_expr_integer_sig visit_expr_integer;
        visit_expr_string_sig visit_expr_string;
        visit_expr_identifier_sig visit_expr_identifier;
        visit_expr_binary_sig visit_expr_binary;
        visit_expr_call_sig visit_expr_call;

        visit_stmt_expr_sig visit_stmt_expr;
        visit_stmt_let_sig visit_stmt_let;
        visit_stmt_block_sig visit_stmt_block;
        visit_stmt_proc_sig visit_stmt_proc;
        visit_stmt_if_sig visit_stmt_if;
        visit_stmt_while_sig visit_stmt_while;
        visit_stmt_return_sig visit_stmt_return;
        visit_stmt_struct_sig visit_stmt_struct;
        visit_stmt_exit_sig visit_stmt_exit;
} visitor;

visitor
visitor_create(void                      *context,
               visit_expr_integer_sig     visit_expr_integer,
               visit_expr_string_sig      visit_expr_string,
               visit_expr_identifier_sig  visit_expr_identifier,
               visit_expr_binary_sig      visit_expr_binary,
               visit_expr_call_sig        visit_expr_call,
               visit_stmt_expr_sig        visit_stmt_expr,
               visit_stmt_let_sig         visit_stmt_let,
               visit_stmt_block_sig       visit_stmt_block,
               visit_stmt_proc_sig        visit_stmt_proc,
               visit_stmt_if_sig          visit_stmt_if,
               visit_stmt_while_sig       visit_stmt_while,
               visit_stmt_return_sig      visit_stmt_return,
               visit_stmt_struct_sig      visit_stmt_struct,
               visit_stmt_exit_sig        visit_stmt_exit);

void *accept_expr_integer(expr *e, visitor *v);
void *accept_expr_string(expr *e, visitor *v);
void *accept_expr_identifier(expr *e, visitor *v);
void *accept_expr_binary(expr *e, visitor *v);
void *accept_expr_call(expr *e, visitor *v);

void *accept_stmt_expr(stmt *s, visitor *v);
void *accept_stmt_let(stmt *s, visitor *v);
void *accept_stmt_block(stmt *s, visitor *v);
void *accept_stmt_proc(stmt *s, visitor *v);
void *accept_stmt_if(stmt *s, visitor *v);
void *accept_stmt_while(stmt *s, visitor *v);
void *accept_stmt_return(stmt *s, visitor *v);
void *accept_stmt_struct(stmt *s, visitor *v);
void *accept_stmt_exit(stmt *s, visitor *v);

#endif // VISITOR_H_INCLUDED
