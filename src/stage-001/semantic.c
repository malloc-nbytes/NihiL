#include "semantic.h"
#include "array.h"
#include "comperr.h"
#include "types.h"
#include "visitor.h"

#include <assert.h>
#include <string.h>

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
        return k == EXPR_KIND_IDENTIFIER
                || k == EXPR_KIND_MEMBER
                || k == EXPR_KIND_DEREF
                || k == EXPR_KIND_INDEX;
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
lookup_type(semantic_checker *checker,
            const token      *id)
{
        binding *bind;

        bind = (binding *)scope_lookup(checker->current,
                                       sv_view(id->lx));

        if (!bind) {
                array_append(checker->errors,
                             comperr_from(str_from_fmt("undeclared type `%s'",
                                                       sv_view(id->lx)),
                                          id->loc));
                return NULL;
        }

        if (bind->k != BINDING_KIND_TYPE) {
                array_append(checker->errors,
                             comperr_from(str_from_fmt("`%s' is not a type",
                                                       sv_view(id->lx)),
                                          id->loc));
                return NULL;
        }

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

static int
is_valid_cast(const type *from,
              const type *to)
{
        return (is_integer_type(from)
                && is_integer_type(to))
                || (from->k == TYPE_KIND_PTR
                    && to->k == TYPE_KIND_PTR);
}

static type *
resolve_type(semantic_checker *checker,
             type             *ty,
             location          loc)
{
        if (ty->k == TYPE_KIND_PTR) {
                type_ptr *ptr;

                ptr = (type_ptr *)ty;
                ptr->to = resolve_type(checker, ptr->to, loc);
                return ty;
        }

        if (ty->k == TYPE_KIND_ARRAY) {
                type_array *array;

                array = (type_array *)ty;
                array->of = resolve_type(checker, array->of, loc);

                return ty;
        }

        if (ty->k == TYPE_KIND_NAMED) {
                type_named *named;
                binding *bind;

                named = (type_named *)ty;
                bind = lookup_type(checker, named->id);

                if (!bind)
                        return (type *)type_void_alloc();

                return bind->ty;
        }

        return ty;
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
        else if (bind->k == BINDING_KIND_TYPE) {
                array_append(checker->errors,
                             comperr_from(str_from_fmt("type `%s' used as a value",
                                                       sv_view(e->id->lx)),
                                          e->id->loc));
                e->base.ty = (type *)type_void_alloc();
        }
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

        if (e->op->k == TOKEN_KIND_PLUS
            || e->op->k == TOKEN_KIND_MINUS) {
                type_ptr *ptr_ty;

                ptr_ty = NULL;

                if (lhs_ty->k == TYPE_KIND_PTR
                    && is_integer_type(rhs_ty)) {
                        ptr_ty = (type_ptr *)lhs_ty;
                } else if (e->op->k == TOKEN_KIND_PLUS
                           && is_integer_type(lhs_ty)
                           && rhs_ty->k == TYPE_KIND_PTR) {
                        ptr_ty = (type_ptr *)rhs_ty;
                } else if (lhs_ty->k == TYPE_KIND_PTR
                           || rhs_ty->k == TYPE_KIND_PTR) {
                        array_append(checker->errors,
                                     comperr_from(str_from_fmt("operator `%s' requires a pointer and an integer operand",
                                                               sv_view(e->op->lx)),
                                                  e->base.loc));

                        e->base.ty = (type *)type_void_alloc();
                        return (void *)e->base.ty;
                }

                if (ptr_ty) {
                        if (ptr_ty->to->k == TYPE_KIND_VOID) {
                                array_append(checker->errors,
                                             comperr_from(str_from("pointer arithmetic on `void*' is not supported"),
                                                          e->base.loc));

                                e->base.ty = (type *)type_void_alloc();
                                return (void *)e->base.ty;
                        }

                        e->base.ty = (type *)ptr_ty;
                        return (void *)e->base.ty;
                }
        }

        if (!type_implements_operator(lhs_ty, e->op->k)) {
                str tystr = type_to_str(lhs_ty);
                array_append(checker->errors,
                             comperr_from(str_from_fmt("type `%s' does not implement operator `%s'",
                                                       tystr.chars, sv_view(e->op->lx)), e->lhs->loc));
                str_destroy(&tystr);
        }

        if (!type_implements_operator(rhs_ty, e->op->k)) {
                str tystr = type_to_str(rhs_ty);
                array_append(checker->errors,
                             comperr_from(str_from_fmt("type `%s' does not implement operator `%s'",
                                                       tystr.chars, sv_view(e->op->lx)), e->rhs->loc));
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
                                                       lhs_ty_str.chars, rhs_ty_str.chars, sv_view(e->op->lx)),
                                          e->lhs->loc));
                str_destroy(&lhs_ty_str);
                str_destroy(&rhs_ty_str);
        }

        e->base.ty = lhs_ty;

        return (void *)e->base.ty;
}

static void *
visit_expr_call(visitor *v, expr_call *e)
{
        semantic_checker *checker;
        type *callee_ty;
        type_proc *proc_ty;
        size_t fixed_count;

        checker = (semantic_checker *)v->context;
        callee_ty = (type *)e->lhs->accept(e->lhs, v);

        if (callee_ty->k != TYPE_KIND_PROC) {
                str callee_tystr;

                callee_tystr = type_to_str(callee_ty);

                array_append(checker->errors,
                             comperr_from(str_from_fmt("cannot call expression of type `%s'",
                                                       callee_tystr.chars),
                                          e->lhs->loc));
                str_destroy(&callee_tystr);

                for (size_t i = 0; i < e->exprs.len; ++i)
                        e->exprs.data[i]->accept(e->exprs.data[i], v);

                e->base.ty = (type *)type_void_alloc();
                return (void *)e->base.ty;
        }

        proc_ty = (type_proc *)callee_ty;
        fixed_count = proc_ty->tys.len;

        if (e->exprs.len < fixed_count
            || (!proc_ty->variadic && e->exprs.len != fixed_count)) {
                array_append(checker->errors,
                             comperr_from(str_from_fmt("procedure expects %zu argument%s but got %zu",
                                                       fixed_count,
                                                       fixed_count == 1 ? "" : "s",
                                                       e->exprs.len),
                                          e->base.loc));
        }

        for (size_t i = 0; i < e->exprs.len; ++i) {
                type *expected_ty;
                type *actual_ty;

                expected_ty = i < fixed_count
                        ? proc_ty->tys.data[i]
                        : NULL;

                actual_ty = visit_expr_expected(v, e->exprs.data[i], expected_ty);

                if (expected_ty && !type_compatible(expected_ty, actual_ty)) {
                        str expected_tystr;
                        str actual_tystr;

                        expected_tystr = type_to_str(expected_ty);
                        actual_tystr = type_to_str(actual_ty);

                        array_append(checker->errors,
                                     comperr_from(str_from_fmt("argument %zu has type `%s' but parameter has type `%s'",
                                                               i+1,
                                                               actual_tystr.chars,
                                                               expected_tystr.chars),
                                                  e->exprs.data[i]->loc));

                        str_destroy(&expected_tystr);
                        str_destroy(&actual_tystr);
                }
        }

        e->base.ty = proc_ty->ty;

        return (void *)e->base.ty;
}

static struct_member *
lookup_struct_member(type_struct *struct_ty,
                    const token  *id)
{
        stmt_struct *declaration;

        declaration = struct_ty->declaration;

        for (size_t i = 0; i < declaration->members.len; ++i) {
                struct_member *member;

                member = &declaration->members.data[i];

                if (tokens_equal(member->id, id))
                        return member;
        }

        return NULL;
}

static void *
visit_expr_struct_literal(visitor *v, expr_struct_literal *e)
{
        semantic_checker *checker;
        binding *bind;
        type *literal_ty;
        type_struct *struct_ty;

        checker = (semantic_checker *)v->context;
        bind = lookup_type(checker, e->for_);

        if (!bind) {
                for (size_t i = 0; i < e->setters.len; ++i)
                        e->setters.data[i].e->accept(e->setters.data[i].e, v);
                e->base.ty = (type *)type_void_alloc();
                return (void *)e->base.ty;
        }

        literal_ty = bind->ty;

        if (literal_ty->k != TYPE_KIND_STRUCT) {
                str tystr;

                tystr = type_to_str(literal_ty);

                array_append(checker->errors,
                             comperr_from(str_from_fmt("`%s' is not a structure type",
                                                       tystr.chars),
                                          e->for_->loc));

                str_destroy(&tystr);

                for (size_t i = 0; i < e->setters.len; ++i)
                        e->setters.data[i].e->accept(e->setters.data[i].e, v);

                e->base.ty = (type *)type_void_alloc();
                return (void *)e->base.ty;
        }

        struct_ty = (type_struct *)literal_ty;

        for (size_t i = 0; i < e->setters.len; ++i) {
                struct_literal_kv *setter;
                struct_member *member;
                type *value_ty;

                setter = &e->setters.data[i];
                member = lookup_struct_member(struct_ty, setter->id);

                if (!member) {
                        array_append(checker->errors,
                                     comperr_from(str_from_fmt("structure `%s' has no member `%s'",
                                                               sv_view(struct_ty->declaration->id->lx),
                                                               sv_view2(setter->id->lx)),
                                                  setter->id->loc));

                        setter->e->accept(setter->e, v);
                        continue;
                }

                for (size_t j = 0; j < i; ++j) {
                        if (tokens_equal(setter->id, e->setters.data[j].id)) {
                                array_append(checker->errors,
                                             comperr_from(str_from_fmt("member `%s' is initialized more than once",
                                                                       sv_view(setter->id->lx)),
                                                          setter->id->loc));
                                break;
                        }
                }

                value_ty = visit_expr_expected(v, setter->e, member->ty);

                if (!type_compatible(member->ty, value_ty)) {
                        str expected_tystr;
                        str actual_tystr;

                        expected_tystr = type_to_str(member->ty);
                        actual_tystr = type_to_str(value_ty);

                        array_append(checker->errors,
                                     comperr_from(str_from_fmt("member %s' has type %s' but requires `%s'",
                                                               sv_view(setter->id->lx),
                                                               actual_tystr.chars,
                                                               expected_tystr.chars),
                                                  setter->id->loc));
                        str_destroy(&expected_tystr);
                        str_destroy(&actual_tystr);
                }

                setter->member = member;
        }

        e->base.ty = literal_ty;

        return (void *)e->base.ty;
}

static void *
visit_expr_member(visitor *v, expr_member *e)
{
        semantic_checker *checker;
        type *lhs_ty;
        type_struct *struct_ty;
        struct_member *member;

        checker = (semantic_checker *)v->context;
        lhs_ty = (type *)e->lhs->accept(e->lhs, v);

        if (lhs_ty->k != TYPE_KIND_STRUCT) {
                str tystr;

                tystr = type_to_str(lhs_ty);

                array_append(checker->errors,
                             comperr_from(str_from_fmt("cannot access member `%s' of expression type `%s'",
                                                       sv_view(e->member->lx),
                                                       tystr.chars),
                                          e->member->loc));

                str_destroy(&tystr);

                e->base.ty = (type *)type_void_alloc();
                return (void *)e->base.ty;
        }

        struct_ty = (type_struct *)lhs_ty;
        member = lookup_struct_member(struct_ty, e->member);

        if (!member) {
                array_append(checker->errors,
                             comperr_from(str_from_fmt("structure `%s' has no member `%s'",
                                                       sv_view(struct_ty->declaration->id->lx),
                                                       sv_view2(e->member->lx)),
                                          e->member->loc));

                e->base.ty = (type *)type_void_alloc();
                return (void *)e->base.ty;
        }

        e->resolved = member;
        e->base.ty = member->ty;

        return (void *)e->base.ty;
}

static void *
visit_expr_deref(visitor *v, expr_deref *e)
{
        semantic_checker *checker;
        type *rhs_ty;
        type_ptr *ptr_ty;

        checker = (semantic_checker *)v->context;
        rhs_ty = (type *)e->rhs->accept(e->rhs, v);

        if (rhs_ty->k != TYPE_KIND_PTR) {
                str tystr;

                tystr = type_to_str(rhs_ty);

                array_append(checker->errors,
                             comperr_from(str_from_fmt("cannot dereference expression of type `%s'",
                                                       tystr.chars),
                                          e->base.loc));

                str_destroy(&tystr);

                e->base.ty = (type *)type_void_alloc();

                return (void *)e->base.ty;
        }

        ptr_ty = (type_ptr *)rhs_ty;
        e->base.ty = ptr_ty->to;

        return (void *)e->base.ty;
}

static void *
visit_expr_address(visitor *v, expr_address *e)
{
        semantic_checker *checker;
        type *rhs_ty;

        checker = (semantic_checker *)v->context;
        rhs_ty = (type *)e->rhs->accept(e->rhs, v);

        if (!is_valid_lvalue(e->rhs->k)) {
                array_append(checker->errors,
                             comperr_from(str_from("cannot take the address of a non-lvalue expression"),
                                         e->base.loc));

                e->base.ty = (type *)type_void_alloc();
                return (void *)e->base.ty;
        }

        e->base.ty = (type *)type_ptr_alloc(rhs_ty, &checker->a);

        return (void *)e->base.ty;
}

static void *
visit_expr_array_literal(visitor *v, expr_array_literal *e)
{
        semantic_checker *checker;
        type *element_ty;
        type_array *array_ty;
        size_t start;

        checker = (semantic_checker *)v->context;
        element_ty = NULL;
        array_ty = NULL;
        start = 0;

        if (checker->expected_type
            && checker->expected_type->k == TYPE_KIND_ARRAY) {
                array_ty = (type_array *)checker->expected_type;
                element_ty = array_ty->of;

                if (e->exprs.len != array_ty->len) {
                        array_append(checker->errors,
                                     comperr_from(str_from_fmt("array literal has %zu element%s but array type requires %zu",
                                                               e->exprs.len,
                                                               e->exprs.len == 1 ? "" : "s",
                                                               array_ty->len),
                                                  e->base.loc));
                }
        } else if (e->exprs.len == 0) {
                array_append(checker->errors,
                             comperr_from(str_from("cannot infer the element type of an empty array literal"),
                                          e->base.loc));

                e->base.ty = (type *)type_void_alloc();
                return (void *)e->base.ty;
        } else {
                element_ty = (type *)e->exprs.data[0]->accept(e->exprs.data[0], v);
                array_ty = type_array_alloc(element_ty, e->exprs.len, &checker->a);
                start = 1;
        }

        for (size_t i = start; i < e->exprs.len; ++i) {
                type *actual_ty;

                actual_ty = visit_expr_expected(v, e->exprs.data[i], element_ty);

                if (!type_compatible(element_ty, actual_ty)) {
                        str expected_tystr;
                        str actual_tystr;

                        expected_tystr = type_to_str(element_ty);
                        actual_tystr = type_to_str(actual_ty);

                        array_append(checker->errors,
                                     comperr_from(str_from_fmt("array element %zu has type %s' but array elements require %s'",
                                                               i + 1,
                                                               actual_tystr.chars,
                                                               expected_tystr.chars),
                                                  e->exprs.data[i]->loc));

                        str_destroy(&expected_tystr);
                        str_destroy(&actual_tystr);
                }
        }

        e->base.ty = (type *)array_ty;

        return (void *)e->base.ty;
}

static void *
visit_expr_index(visitor *v, expr_index *e)
{
        semantic_checker *checker;
        type *lhs_ty;
        type *idx_ty;

        checker = (semantic_checker *)v->context;
        lhs_ty = (type *)e->lhs->accept(e->lhs, v);
        idx_ty = (type *)e->idx->accept(e->idx, v);

        if (!is_integer_type(idx_ty)) {
                str tystr;

                tystr = type_to_str(idx_ty);

                array_append(checker->errors,
                             comperr_from(str_from_fmt("cannot index with expression of type `%s'",
                                                       tystr.chars),
                                          e->idx->loc));

                str_destroy(&tystr);
        }

        if (lhs_ty->k == TYPE_KIND_ARRAY) {
                e->base.ty = ((type_array *)lhs_ty)->of;
                return (void *)e->base.ty;
        }

        if (lhs_ty->k == TYPE_KIND_PTR) {
                e->base.ty = ((type_ptr *)lhs_ty)->to;
                return (void *)e->base.ty;
        }

        { // bad case
                str tystr;

                tystr = type_to_str(lhs_ty);

                array_append(checker->errors,
                             comperr_from(str_from_fmt("cannot index expression of type `%s'",
                                                       tystr.chars),
                                          e->lhs->loc));

                str_destroy(&tystr);
        }

        e->base.ty = (type *)type_void_alloc();

        return (void *)e->base.ty;
}

static void *
visit_expr_cast(visitor *v, expr_cast *e)
{
        semantic_checker *checker;
        type *from;

        checker = (semantic_checker *)v->context;
        e->to = resolve_type(checker, e->to, e->base.loc);

        from = (type *)e->rhs->accept(e->rhs, v);

        if (e->to->k == TYPE_KIND_VOID) {
                array_append(checker->errors,
                             comperr_from(str_from("cannot cast to `void'"),
                                          e->base.loc));
                e->base.ty = (type *)type_void_alloc();
                return (void *)e->base.ty;
        }

        if (!is_valid_cast(from, e->to)) {
                str fromstr;
                str tostr;

                fromstr = type_to_str(from);
                tostr = type_to_str(e->to);

                array_append(checker->errors,
                             comperr_from(str_from_fmt("cannot cast `%s' to `%s'",
                                                       fromstr.chars,
                                                       tostr.chars),
                                          e->base.loc));

                str_destroy(&fromstr);
                str_destroy(&tostr);

                e->base.ty = (type *)type_void_alloc();
                return (void *)e->base.ty;
        }

        e->base.ty = e->to;

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

        s->ty = resolve_type(checker, s->ty, s->id->loc);

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

        s->ty = resolve_type(checker, s->ty, s->id->loc);

        for (size_t i = 0; i < s->params.len; ++i) {
                proc_param *param;

                param = &s->params.data[i];
                param->ty = resolve_type(checker, param->ty, param->id->loc);
        }

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
        semantic_checker *checker;

        checker = (semantic_checker *)v->context;

        s->e->accept(s->e, v);

        if (s->then->k != STMT_KIND_BLOCK) {
                array_append(checker->errors,
                             comperr_from(str_from("the body of `if' statements must be a block"),
                                          s->then->loc));
        }

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
                             comperr_from(str_from("return statement outside a procedure"),
                                          s->base.loc));
                if (s->e)
                        s->e->accept(s->e, v);
                return NULL;
        }

        if (checker->expected_type->k == TYPE_KIND_NEVER) {
                array_append(checker->errors,
                             comperr_from(str_from("cannot return from a `never' function"),
                                          s->base.loc));
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

static void *
visit_stmt_struct(visitor *v, stmt_struct *s)
{
        semantic_checker *checker;

        checker = (semantic_checker *)v->context;

        s->ty = type_struct_alloc(s, &checker->a);
        s->bind = declare(checker, s->id, BINDING_KIND_TYPE, (type *)s->ty, s);

        for (size_t i = 0; i < s->members.len; ++i) {
                struct_member *member;

                member = &s->members.data[i];
                member->ty = resolve_type(checker, member->ty, member->id->loc);

                if (member->ty->k == TYPE_KIND_VOID) {
                        array_append(checker->errors,
                                     comperr_from(str_from("structure members may not be of type `void'"),
                                                  member->id->loc));
                }

                if (member->ty == (type *)s->ty) {
                        array_append(checker->errors,
                                     comperr_from(str_from("cannot have a structure of infinite size"),
                                                  member->id->loc));
                }

                for (size_t j = 0; j < i; ++j) {
                        if (tokens_equal(member->id, s->members.data[j].id)) {
                                array_append(checker->errors,
                                             comperr_from(str_from_fmt("duplicate member `%s'",
                                                                       sv_view(member->id->lx)),
                                                          member->id->loc));
                                break;
                        }
                }
        }

        return NULL;
}

static void *
visit_stmt_exit(visitor *v, stmt_exit *s)
{
        semantic_checker *checker;

        checker = (semantic_checker *)v->context;

        if (!checker->expected_type) {
                array_append(checker->errors,
                             comperr_from(str_from("exit statement outside a procedure"),
                                          s->base.loc));
        }

        if (s->e)
                s->e->accept(s->e, v);

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
                           visit_expr_call,
                           visit_expr_struct_literal,
                           visit_expr_member,
                           visit_expr_deref,
                           visit_expr_address,
                           visit_expr_array_literal,
                           visit_expr_index,
                           visit_expr_cast,
                           visit_stmt_expr,
                           visit_stmt_let,
                           visit_stmt_block,
                           visit_stmt_proc,
                           visit_stmt_if,
                           visit_stmt_while,
                           visit_stmt_return,
                           visit_stmt_struct,
                           visit_stmt_exit);

        for (size_t i = 0; i < c.p->stmts.len; ++i)
                c.p->stmts.data[i]->accept(c.p->stmts.data[i], &v);

        return c;
}
