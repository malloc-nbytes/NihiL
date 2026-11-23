#include "grammar.h"
#include "arena.h"
#include "visitor.h"

expr_integer *
expr_integer_alloc(const token *i, arena *a)
{
        expr_integer *e;

        e = (expr_integer *)arena_alloc(a, sizeof(*e));
        e->i = i;
        e->base.k = EXPR_KIND_INTEGER_LITERAL;
        e->base.loc = i->loc;
        e->base.ty = NULL;
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
        e->base.ty = NULL;
        e->base.accept = accept_expr_string;

        return e;
}

expr_identifier *
expr_identifier_alloc(const token *id, arena *a)
{
        expr_identifier *e;

        e = (expr_identifier *)arena_alloc(a, sizeof(*e));
        e->id = id;
        e->bind = NULL;
        e->base.k = EXPR_KIND_IDENTIFIER;
        e->base.loc = id->loc;
        e->base.ty = NULL;
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
        e->base.ty = NULL;
        e->base.accept = accept_expr_binary;

        return e;
}

expr_call *
expr_call_alloc(expr     *lhs,
                exprp_ar  exprs,
                arena    *a)
{
        expr_call *e;

        e = (expr_call *)arena_alloc(a, sizeof(*e));
        e->lhs = lhs;
        e->exprs = exprs;
        e->base.k = EXPR_KIND_CALL;
        e->base.loc = lhs->loc;
        e->base.ty = NULL;
        e->base.accept = accept_expr_call;

        return e;
}

expr_struct_literal *
expr_struct_literal_alloc(const token          *for_,
                          struct_literal_kv_ar  setters,
                          arena                *a)
{
        expr_struct_literal *e;

        e = (expr_struct_literal *)arena_alloc(a, sizeof(*e));
        e->for_ = for_;
        e->setters = setters;
        e->base.k = EXPR_KIND_STRUCT_LITERAL;
        e->base.loc = for_->loc;
        e->base.ty = NULL;
        e->base.accept = accept_expr_struct_literal;

        return e;
}

expr_member *
expr_member_alloc(expr        *lhs,
                  const token *member,
                  arena       *a)
{
        expr_member *e;

        e = (expr_member *)arena_alloc(a, sizeof(*e));
        e->lhs = lhs;
        e->member = member;
        e->resolved = NULL;
        e->base.k = EXPR_KIND_MEMBER;
        e->base.loc = lhs->loc;
        e->base.ty = NULL;
        e->base.accept = accept_expr_member;

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
        s->bind = NULL;
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
        s->bind = NULL;
        s->base.k = STMT_KIND_PROC;
        s->base.loc = loc;
        s->base.accept = accept_stmt_proc;

        return s;
}

stmt_if *
stmt_if_alloc(expr     *e,
              stmt     *then,
              stmt     *else_,
              location  loc,
              arena    *a)
{
        stmt_if *s;

        s = (stmt_if *)arena_alloc(a, sizeof(*s));
        s->e = e;
        s->then = then;
        s->else_ = else_;
        s->base.k = STMT_KIND_IF;
        s->base.loc = loc;
        s->base.accept = accept_stmt_if;

        return s;
}

stmt_while *
stmt_while_alloc(expr     *e,
                 stmt     *body,
                 location  loc,
                 arena    *a)
{
        stmt_while *s;

        s = (stmt_while *)arena_alloc(a, sizeof(*s));
        s->e = e;
        s->body = body;
        s->base.k = STMT_KIND_WHILE;
        s->base.loc = loc;
        s->base.accept = accept_stmt_while;

        return s;
}

stmt_return *
stmt_return_alloc(expr *e, location loc, arena *a)
{
        stmt_return *s;

        s = (stmt_return *)arena_alloc(a, sizeof(*s));
        s->e = e;
        s->base.k = STMT_KIND_RETURN;
        s->base.loc = loc;
        s->base.accept = accept_stmt_return;

        return s;
}

stmt_struct *
stmt_struct_alloc(const token      *id,
                  struct_member_ar  members,
                  location          loc,
                  arena            *a)
{
        stmt_struct *s;

        s = (stmt_struct *)arena_alloc(a, sizeof(*s));
        s->id = id;
        s->members = members;
        s->bind = NULL;
        s->ty = NULL;
        s->base.k = STMT_KIND_STRUCT;
        s->base.loc = loc;
        s->base.accept = accept_stmt_struct;

        return s;
}

stmt_exit *
stmt_exit_alloc(expr *e, location loc, arena *a)
{
        stmt_exit *s;

        s = (stmt_exit *)arena_alloc(a, sizeof(*s));
        s->e = e;
        s->base.k = STMT_KIND_EXIT;
        s->base.loc = loc;
        s->base.accept = accept_stmt_exit;

        return s;
}
