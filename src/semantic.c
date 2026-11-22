#include "semantic.h"
#include "array.h"
#include "comperr.h"
#include "types.h"
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

static int
is_valid_lvalue(expr_kind k)
{
        return k == EXPR_KIND_IDENTIFIER;
}

static int
operator_will_assign(token_kind k)
{
        return k == TOKEN_KIND_EQUALS
                || k == TOKEN_KIND_PLUS_EQUALS
                || k == TOKEN_KIND_MINUS_EQUALS
                || k == TOKEN_KIND_ASTERISK_EQUALS
                || k == TOKEN_KIND_FSLASH_EQUALS
                || k == TOKEN_KIND_PERCENT_EQUALS
                || k == TOKEN_KIND_PIPE_EQUALS
                || k == TOKEN_KIND_AMPERSAND_EQUALS
                || k == TOKEN_KIND_TILDE_EQUALS;
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

        return bind;
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

static type *
visit_expr_expected(visitor *v,
                    expr    *e,
                    type    *expected_type)
{
        semantic_checker *checker;
        type *old_expected_type;
        type *actual_type;

        checker = (semantic_checker *)v->context;
        old_expected_type = checker->expected_type;
        checker->expected_type = expected_type;

        actual_type = (type *)e->accept(e, v);

        checker->expected_type = old_expected_type;

        return actual_type;
}

static int
is_integer_type(const type *ty)
{
        return ty
                && ty->k >= TYPE_KIND_I8
                && ty->k < __TYPE_INTEGERS;
}

static void *
visit_expr_integer(visitor *v, expr_integer *e)
{
        semantic_checker *checker;

        checker = (semantic_checker *)v->context;

        if (is_integer_type(checker->expected_type))
                e->base.ty = checker->expected_type;
        else
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
        lhs_ty = visit_expr_expected(v, e->lhs, checker->expected_type);
        rhs_ty = visit_expr_expected(v, e->rhs, lhs_ty);

        if (!type_implements_operator(lhs_ty, e->op->k)) {
                str tystr = type_to_str(lhs_ty);
                array_append(checker->errors,
                             comperr_from(str_from_fmt("type `%s' does not implement operator `%s'",
                                                       tystr, sv_view(e->op->lx)), e->lhs->loc));
                str_destroy(&tystr);
        }

        if (!type_implements_operator(rhs_ty, e->op->k)) {
                str tystr = type_to_str(rhs_ty);
                array_append(checker->errors,
                             comperr_from(str_from_fmt("type `%s' does not implement operator `%s'",
                                                       tystr, sv_view(e->op->lx)), e->rhs->loc));
                str_destroy(&tystr);
        }

        if (operator_will_assign(e->op->k)
            && !is_valid_lvalue(e->lhs->k)) {
                array_append(checker->errors,
                             comperr_from(str_from_fmt("performing an assignment `%s' on an invalid lvalue",
                                                       token_kind_to_ccstr(e->op->k)),
                                          e->lhs->loc));
        }

        if (!type_compatible(lhs_ty, rhs_ty)) {
                str lhs_ty_str = type_to_str(lhs_ty);
                str rhs_ty_str = type_to_str(rhs_ty);
                array_append(checker->errors,
                             comperr_from(str_from_fmt("types `%s' and `%s' are not compatible with operator `%s'",
                                                       lhs_ty_str, rhs_ty_str, sv_view(e->op->lx)),
                                          e->lhs->loc));
                str_destroy(&lhs_ty_str);
                str_destroy(&rhs_ty_str);
        }

        e->base.ty = lhs_ty;

        return (void *)e->base.ty;
}

static void *
visit_stmt_expr(visitor *v, stmt_expr *s)
{
        s->e->accept(s->e, v);

        return NULL;
}

static void *
visit_stmt_let(visitor *v, stmt_let *s)
{
        semantic_checker *checker;

        checker = (semantic_checker *)v->context;

        if (s->ty->k == TYPE_KIND_VOID) {
                array_append(checker->errors,
                             comperr_from(str_from("variables may not be of type `void'"),
                                          s->base.loc));
        }

        if (s->e) {
                type *init_ty;

                init_ty = visit_expr_expected(v, s->e, s->ty);

                if (!type_compatible(s->ty, init_ty)) {
                        str let_tystr = type_to_str(s->ty);
                        str init_tystr = type_to_str(init_ty);
                        array_append(checker->errors, comperr_from(str_from_fmt("types `%s' and `%s' are not compatible",
                                                                                let_tystr.chars, init_tystr.chars),
                                                                   s->base.loc));
                        str_destroy(&let_tystr);
                        str_destroy(&init_tystr);
                }
        }

        s->bind = declare(checker, s->id, BINDING_KIND_VARIABLE, s->ty, s);

        return NULL;
}

static void *
visit_stmt_block(visitor *v, stmt_block *s)
{
        semantic_checker *checker;

        checker = (semantic_checker *)v->context;
        scope_push(checker);

        for (size_t i = 0; i < s->stmts.len; ++i)
                s->stmts.data[i]->accept(s->stmts.data[i], v);

        scope_pop(checker);

        return NULL;
}

static void *
visit_stmt_proc(visitor *v, stmt_proc *s)
{
        semantic_checker *checker;
        typep_ar param_tys;
        type_proc *proc_ty;
        type *old_expected_type;

        checker = (semantic_checker *)v->context;
        param_tys = array_empty_explicit(typep_ar);

        for (size_t i = 0; i < s->params.len; ++i)
                array_append(param_tys, (type *)s->params.data[i].ty);

        proc_ty = type_proc_alloc(s->ty, param_tys, s->flags & PROC_FLAG_VARIADIC, &checker->a);

        s->bind = declare(checker,
                          s->id,
                          BINDING_KIND_PROCEDURE,
                          (type *)proc_ty,
                          s);

        if (!s->body)
                return NULL;

        scope_push(checker);

        for (size_t i = 0; i < s->params.len; ++i) {
                proc_param *param;

                param = &s->params.data[i];

                if (param->ty->k == TYPE_KIND_VOID) {
                        array_append(checker->errors,
                                     comperr_from(str_from("procedure parameters must not be of type `void'"),
                                                  param->id->loc));
                }

                param->bind = declare(checker,
                                      param->id,
                                      BINDING_KIND_PARAMETER,
                                      (type *)param->ty,
                                      param);
        }

        old_expected_type = checker->expected_type;
        checker->expected_type = s->ty;
        s->body->base.accept((stmt *)s->body, v);
        checker->expected_type = old_expected_type;

        scope_pop(checker);

        return NULL;
}

static void *
visit_stmt_if(visitor *v, stmt_if *s)
{
        s->e->accept(s->e, v);
        s->then->accept(s->then, v);
        if (s->else_)
                s->else_->accept(s->else_, v);

        return NULL;
}

static void *
visit_stmt_while(visitor *v, stmt_while *s)
{
        s->e->accept(s->e, v);
        s->body->accept(s->body, v);

        return NULL;
}

static void *
visit_stmt_return(visitor *v, stmt_return *s)
{
        semantic_checker *checker;
        type *return_ty;

        checker = (semantic_checker *)v->context;

        if (!checker->expected_type) {
                array_append(checker->errors,
                             comperr_from(str_from("return statemesnt outside a procedure"),
                                          s->base.loc));
                if (s->e)
                        s->e->accept(s->e, v);
                return NULL;
        }

        if (!s->e) {
                if (checker->expected_type->k != TYPE_KIND_VOID) {
                        array_append(checker->errors,
                                     comperr_from(str_from("non-void procedure must return a value"),
                                                  s->base.loc));
                }
                return NULL;
        }

        return_ty = visit_expr_expected(v, s->e, checker->expected_type);

        if (!type_compatible(checker->expected_type, return_ty)) {
                str expected_ty;
                str actual_ty;

                expected_ty = type_to_str(checker->expected_type);
                actual_ty = type_to_str(return_ty);

                array_append(checker->errors,
                             comperr_from(str_from_fmt("cannot return `%s' from procedure returning `%s'",
                                                       actual_ty.chars, expected_ty.chars),
                                          s->base.loc));
                str_destroy(&expected_ty);
                str_destroy(&actual_ty);
        }

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
                .global = NULL,
                .current = NULL,
                .expected_type = NULL,
                .a = (arena) {0},
        };

        arena_init(&c.a, ARENA_DEFAULT_BLK_SIZE);
        c.global = scope_create(NULL, &c.a);
        c.current = c.global;

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

        for (size_t i = 0; i < c.p->stmts.len; ++i)
                c.p->stmts.data[i]->accept(c.p->stmts.data[i], &v);

        return c;
}
