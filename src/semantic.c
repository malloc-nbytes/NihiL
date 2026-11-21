#include "semantic.h"
#include "visitor.h"

static void
scope_push(semantic_checker *checker)
{
        checker->current = scope_create(checker->current, &checker->a);
}

static void
scope_pop(semantic_checker *checker)
{
        scope *old;

        old = checker->current;
        checker->current = old->parent;
        scope_destroy(old);
}

static void *
visit_expr_integer(visitor *v, expr_integer *e)
{
        (void)v;
        (void)e;
        return NULL;
}

static void *
visit_expr_string(visitor *v, expr_string *e)
{
        (void)v;
        (void)e;
        return NULL;
}

static void *
visit_expr_identifier(visitor *v, expr_identifier *e)
{
        (void)v;
        (void)e;
        return NULL;
}

static void *
visit_expr_binary(visitor *v, expr_binary *e)
{
        (void)v;
        (void)e;
        return NULL;
}

static void *
visit_stmt_expr(visitor *v, stmt_expr *s)
{
        (void)v;
        (void)s;
        return NULL;
}

static void *
visit_stmt_let(visitor *v, stmt_let *s)
{
        (void)v;
        (void)s;
        return NULL;
}

static void *
visit_stmt_block(visitor *v, stmt_block *s)
{
        (void)v;
        (void)s;
        return NULL;
}

static void *
visit_stmt_proc(visitor *v, stmt_proc *s)
{
        (void)v;
        (void)s;
        return NULL;
}

static void *
visit_stmt_if(visitor *v, stmt_if *s)
{
        (void)v;
        (void)s;
        return NULL;
}

static void *
visit_stmt_while(visitor *v, stmt_while *s)
{
        (void)v;
        (void)s;
        return NULL;
}

semantic_checker
semantic_analysis(MOVE(parser *p))
{
        semantic_checker c;
        visitor v;

        c = (semantic_checker) {
                .p = p,
                .errors = array_empty(),
                .bindings = array_empty(),
                .global = scope_create(NULL, &c.a),
                .current = NULL,
                .expected_type = NULL,
                .a = (arena) {0},
        };

        v = visitor_create(&c,
                           visit_expr_integer,
                           visit_expr_string,
                           visit_expr_identifier,
                           visit_expr_binary,
                           visit_stmt_expr,
                           visit_stmt_let,
                           visit_stmt_block,
                           visit_stmt_proc,
                           visit_stmt_if,
                           visit_stmt_while);

        arena_init(&c.a, ARENA_DEFAULT_BLK_SIZE);

        for (size_t i = 0; i < c.p->stmts.len; ++i)
                c.p->stmts.data[i]->accept(c.p->stmts.data[i], &v);

        return c;
}
