#include "grammar.h"
#include "visitor.h"

expr_integer *
expr_integer_alloc(const token *i, arena *a)
{
        expr_integer *e;

        e = (expr_integer *)arena_alloc(a, sizeof(*e));
        e->i = i;
        e->base.k = EXPR_KIND_INTEGER_LITERAL;
        e->base.loc = i->loc;
        e->base.accept = accept_expr_integer;

        return e;
}

expr_string *
expr_string_alloc(const token *s, arena *a)
{
        expr_string *e;

        e = (expr_string *)arena_alloc(a, sizeof(*e));
        e->s = s;
        e->base.k = EXPR_KIND_STRING_LITERAL;
        e->base.loc = s->loc;
        e->base.accept = accept_expr_string;

        return e;
}

expr_identifier *
expr_identifier_alloc(const token *id, arena *a)
{
        expr_identifier *e;

        e = (expr_identifier *)arena_alloc(a, sizeof(*e));
        e->id = id;
        e->base.k = EXPR_KIND_IDENTIFIER;
        e->base.loc = id->loc;
        e->base.accept = accept_expr_identifier;

        return e;
}

expr_binary *
expr_binary_alloc(expr        *lhs,
                  const token *op,
                  expr        *rhs,
                  arena       *a)
{
        expr_binary *e;

        e = (expr_binary *)arena_alloc(a, sizeof(*e));
        e->lhs = lhs;
        e->op = op;
        e->rhs = rhs;
        e->base.k = EXPR_KIND_BINARY;
        e->base.loc = lhs->loc;
        e->base.accept = accept_expr_binary;

        return e;
}

stmt_expr *
stmt_expr_alloc(expr *e, arena *a)
{
        stmt_expr *s;

        s = (stmt_expr *)arena_alloc(a, sizeof(*s));
        s->e = e;
        s->base.k = STMT_KIND_EXPR;
        s->base.loc = e->loc;
        s->base.accept = accept_stmt_expr;

        return s;
}

stmt_let *
stmt_let_alloc(const token *id,
               type        *ty,
               expr        *e,
               location     loc,
               arena       *a)
{
        stmt_let *s;

        s = (stmt_let *)arena_alloc(a, sizeof(*s));
        s->id = id;
        s->ty = ty;
        s->e = e;
        s->base.k = STMT_KIND_LET;
        s->base.loc = loc;
        s->base.accept = accept_stmt_let;

        return s;
}

stmt_block *
stmt_block_alloc(stmtp_ar stmts, location loc, arena *a)
{
        stmt_block *s;

        s = (stmt_block *)arena_alloc(a, sizeof(*s));
        s->stmts = stmts;
        s->base.k = STMT_KIND_BLOCK;
        s->base.loc = loc;
        s->base.accept = accept_stmt_block;

        return s;
}

stmt_proc *
stmt_proc_alloc(uint32_t       flags,
                const token   *id,
                proc_param_ar  params,
                type          *ty,
                stmt_block    *body,
                location       loc,
                arena         *a)
{
        stmt_proc *s;

        s = (stmt_proc *)arena_alloc(a, sizeof(*s));
        s->flags = flags;
        s->id = id;
        s->params = params;
        s->ty = ty;
        s->body = body;
        s->base.k = STMT_KIND_PROC;
        s->base.loc = loc;
        s->base.accept = accept_stmt_proc;

        return s;
}
