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

static binding *
declare(semantic_checker *checker,
        const token      *id,
        binding_kind      k,
        type             *ty,
        void             *declaration)
{
        binding *bind;
        const char *name;

        name = sv_view(id->lx);

        if (scope_lookup_local(checker->current, name)) {
                array_append(checker->errors,
                             comperr_from(str_from_fmt("duplicate identifier of `%s'", name),
                                          id->loc));
                return NULL;
        }

        bind = (binding *)arena_alloc(&checker->a, sizeof(*bind));
        bind->k = k;
        bind->id = id;
        bind->ty = ty;
        bind->declaration = declaration;

        array_append(checker->bindings, bind);
        scope_bind(checker->current, name, bind, &checker->a);
}

static binding *
lookup(semantic_checker *checker,
       const token      *id)
{
        binding *bind;

        bind = NULL;

        if (!(bind = (binding *)scope_lookup(checker->current, sv_view(id->lx))))
                array_append(checker->errors,
                             comperr_from(str_from_fmt("undeclared identifier `%s'", sv_view(id->lx)),
                                          id->loc));

        return bind;
}

static void *
visit_expr_integer(visitor *v, expr_integer *e)
{
        e->base.ty = (type *)type_i32_alloc();

        return (void *)e->base.ty;
}

static void *
visit_expr_string(visitor *v, expr_string *e)
{
        semantic_checker *checker;

        checker = (semantic_checker *)v->context;

        e->base.ty = (type *)type_ptr_alloc((type *)type_u8_alloc(), &checker->a);

        return (void *)e->base.ty;
}

static void *
visit_expr_identifier(visitor *v, expr_identifier *e)
{
        semantic_checker *checker;
        binding *bind;

        checker = (semantic_checker *)v->context;

        if (!(bind = lookup(checker, e->id)))
                e->base.ty = (type *)type_void_alloc();
        else
                e->base.ty = bind->ty;

        e->bind = bind;

        return (void *)e->base.ty;
}

static void *
visit_expr_binary(visitor *v, expr_binary *e)
{
        semantic_checker *checker;
        type *lhs_ty;
        type *rhs_ty;

        checker = (semantic_checker *)v->context;
        lhs_ty = e->lhs->accept(e->lhs, v);
        rhs_ty = e->rhs->accept(e->rhs, v);

        if (!type_implements_operator(lhs_ty, e->op->k)) {
                str tystr = type_to_str(lhs_ty);
                array_append(checker->errors,
                             comperr_from(str_from_fmt("type `%s' does not implement operator `%s'",
                                                       tystr, sv_view(e->op->lx)), e->lhs->loc));
        }
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

static void *
visit_stmt_return(visitor *v, stmt_return *s)
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
                           visit_stmt_while,
                           visit_stmt_return);

        arena_init(&c.a, ARENA_DEFAULT_BLK_SIZE);

        for (size_t i = 0; i < c.p->stmts.len; ++i)
                c.p->stmts.data[i]->accept(c.p->stmts.data[i], &v);

        return c;
}
