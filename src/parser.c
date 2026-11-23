#include "parser.h"
#include "grammar.h"
#include "lexer.h"
#include "keywords.h"
#include "error.h"

#include <assert.h>
#include <string.h>

static stmt *parse_stmt(parser *p);
static expr *parse_expr(parser *p);

static token *
expect(parser *p, token_kind k)
{
        const token *hd;

        if ((hd = lexer_peek(p->l))->k != k) {
                p->error = comperr_from(str_from_fmt("expected `%s' but got `%s'",
                                                     token_kind_to_ccstr(k),
                                                     token_kind_to_ccstr(hd->k)),
                                        hd->loc);
                return NULL;
        }

        return lexer_next(p->l);
}

static token *
expect_keyword(parser *p, const char *kw)
{
        const token *hd;

        if ((hd = lexer_peek(p->l))->k != TOKEN_KIND_KEYWORD) {
                p->error = comperr_from(str_from_fmt("expected keyword `%s' but got `%s'",
                                                     kw, token_kind_to_ccstr(hd->k)),
                                        hd->loc);
                return NULL;
        }

        if (strcmp(sv_view(hd->lx), kw)) {
                p->error = comperr_from(str_from_fmt("expected keyword `%s' but got `%s'",
                                                     kw, sv_view(hd->lx)),
                                        hd->loc);
                return NULL;
        }

        return lexer_next(p->l);
}

static int
parse_struct_literal_kv(parser               *p,
                        struct_literal_kv_ar *o_setters)
{
        *o_setters = array_empty_explicit(struct_literal_kv_ar);

        if (!expect(p, TOKEN_KIND_LCURLY))
                return 0;

        while (lexer_peek(p->l)->k != TOKEN_KIND_RCURLY
               && lexer_peek(p->l)->k != TOKEN_KIND_EOF) {
                const token *id;
                expr *e;

                if (!expect(p, TOKEN_KIND_PERIOD))
                        return 0;
                if (!(id = expect(p, TOKEN_KIND_IDENTIFIER)))
                        return 0;
                if (!expect(p, TOKEN_KIND_EQUALS))
                        return 0;
                if (!(e = parse_expr(p)))
                        return 0;

                array_append(*o_setters, ((struct_literal_kv) {
                        .id = id,
                        .e = e,
                        .member = NULL,
                }));

                if (lexer_peek(p->l)->k != TOKEN_KIND_COMMA)
                        break;

                lexer_discard(p->l); // ,
        }

        if (!expect(p, TOKEN_KIND_RCURLY))
                return 0;

        return 1;
}

static expr *
parse_primary_expr(parser *p)
{
        const token *hd = lexer_peek(p->l);

        switch (hd->k) {
        case TOKEN_KIND_INTEGER_LITERAL: {
                lexer_discard(p->l);
                return (expr *)expr_integer_alloc(hd, &p->a);
        } break;
        case TOKEN_KIND_IDENTIFIER:
                if (lexer_peekn(p->l, 1)->k == TOKEN_KIND_LCURLY) {
                        const token *for_;
                        struct_literal_kv_ar setters;

                        for_ = lexer_next(p->l);

                        if (!parse_struct_literal_kv(p, &setters))
                                return NULL;

                        return (expr *)expr_struct_literal_alloc(for_, setters, &p->a);
                }
                lexer_discard(p->l);
                return (expr *)expr_identifier_alloc(hd, &p->a);
        case TOKEN_KIND_STRING_LITERAL:
                lexer_discard(p->l);
                return (expr *)expr_string_alloc(hd, &p->a);
        case TOKEN_KIND_LPAREN: {
                expr *e;

                lexer_discard(p->l); // (

                if (!(e = parse_expr(p)))
                        return NULL;

                if (!expect(p, TOKEN_KIND_RPAREN))
                        return NULL;

                return e;
        } break;
        default:
                break;
        }

        p->error = comperr_from(str_from("invalid primary expression"), hd->loc);

        return NULL;
}

static expr *
parse_call_expr(parser *p)
{
        expr *lhs;

        if (!(lhs = parse_primary_expr(p)))
                return NULL;

        while (lexer_peek(p->l)->k == TOKEN_KIND_LPAREN) {
                exprp_ar exprs = array_empty();

                lexer_discard(p->l); // (

                if (lexer_peek(p->l)->k != TOKEN_KIND_RPAREN
                    && lexer_peek(p->l)->k != TOKEN_KIND_EOF) {
                        while (1) {
                                expr *arg;

                                if (!(arg = parse_expr(p)))
                                        return NULL;

                                array_append(exprs, arg);

                                if (lexer_peek(p->l)->k != TOKEN_KIND_COMMA)
                                        break;

                                lexer_discard(p->l); // ,
                        }
                }

                if (!expect(p, TOKEN_KIND_RPAREN))
                        return NULL;

                lhs = (expr *)expr_call_alloc(lhs, exprs, &p->a);
        }

        return lhs;
}

static expr *
parse_unary_expr(parser *p)
{
        // TODO
        return parse_call_expr(p);
}

static expr *
parse_multiplicative_expr(parser *p)
{
        expr *lhs;
        const token *cur;

        lhs = parse_unary_expr(p);
        cur = lexer_peek(p->l);

        while (cur && (cur->k == TOKEN_KIND_ASTERISK
                        || cur->k == TOKEN_KIND_FSLASH)) {
                token *op;
                expr *rhs;
                expr_binary *bin;

                op = lexer_next(p->l);
                if (!(rhs = parse_unary_expr(p)))
                        return NULL;
                bin = expr_binary_alloc(lhs, op, rhs, &p->a);
                bin->base.loc = lhs->loc;
                lhs = (expr *)bin;
                cur = lexer_peek(p->l);
        }

        return lhs;
}

static expr *
parse_additive_expr(parser *p)
{
        expr *lhs;
        const token *cur;

        lhs = parse_multiplicative_expr(p);
        cur = lexer_peek(p->l);

        while (cur && (cur->k == TOKEN_KIND_PLUS
                        || cur->k == TOKEN_KIND_MINUS)) {
                token *op;
                expr *rhs;
                expr_binary *bin;

                op = lexer_next(p->l);
                if (!(rhs = parse_multiplicative_expr(p)))
                        return NULL;
                bin = expr_binary_alloc(lhs, op, rhs, &p->a);
                bin->base.loc = lhs->loc;
                lhs = (expr *)bin;
                cur = lexer_peek(p->l);
        }

        return lhs;
}

static expr *
parse_equalitive_expr(parser *p)
{
        expr *lhs;
        const token *cur;

        lhs = parse_additive_expr(p);
        cur = lexer_peek(p->l);

        while (cur && (cur->k == TOKEN_KIND_DOUBLE_EQUALS
                        || cur->k == TOKEN_KIND_BANG_EQUALS
                        || cur->k == TOKEN_KIND_GREATERTHAN_EQUALS
                        || cur->k == TOKEN_KIND_LESSTHAN_EQUALS
                        || cur->k == TOKEN_KIND_LESSTHAN
                        || cur->k == TOKEN_KIND_GREATERTHAN)) {
                token *op;
                expr *rhs;
                expr_binary *bin;

                op = lexer_next(p->l);
                if (!(rhs = parse_additive_expr(p)))
                        return NULL;
                bin = expr_binary_alloc(lhs, op, rhs, &p->a);
                bin->base.loc = lhs->loc;
                lhs = (expr *)bin;
                cur = lexer_peek(p->l);
        }

        return lhs;
}

static expr *
parse_logical_expr(parser *p)
{
        expr *lhs;
        const token *cur;

        lhs = parse_equalitive_expr(p);
        cur = lexer_peek(p->l);

        while (cur && (cur->k == TOKEN_KIND_DOUBLE_PIPE
                        || cur->k == TOKEN_KIND_DOUBLE_AMPERSAND)) {
                token *op;
                expr *rhs;
                expr_binary *bin;

                op = lexer_next(p->l);
                if (!(rhs = parse_equalitive_expr(p)))
                        return NULL;
                bin = expr_binary_alloc(lhs, op, rhs, &p->a);
                bin->base.loc = lhs->loc;
                lhs = (expr *)bin;
                cur = lexer_peek(p->l);
        }

        return lhs;
}

static expr *
parse_assignment_expr(parser *p)
{
        expr *lhs;
        const token *cur;

        lhs = parse_logical_expr(p);

        cur = lexer_peek(p->l);
        if (cur->k == TOKEN_KIND_EQUALS) {
                token *op;
                expr *rhs;

                op = lexer_next(p->l);
                if (!(rhs = parse_assignment_expr(p)))
                        return NULL;
                return (expr *)expr_binary_alloc(lhs, op, rhs, &p->a);
        }

        return lhs;
}

static expr *
parse_expr(parser *p)
{
        return parse_assignment_expr(p);
}

static type *
parse_type(parser *p)
{
        const token *hd;
        const char *lx;
        type *base;

        if (lexer_peek(p->l)->k == TOKEN_KIND_IDENTIFIER) {
                hd = lexer_next(p->l);
                base = (type *)type_named_alloc(hd, &p->a);
        } else {
                hd = lexer_next(p->l);

                if (hd->k == TOKEN_KIND_BANG) {
                        base = (type *)type_never_alloc(&p->a);
                        goto ptrs;
                }

                if (hd->k != TOKEN_KIND_TYPE) {
                        p->error = comperr_from(str_from_fmt("expected `type' but got `%s'",
                                                             token_kind_to_ccstr(hd->k)),
                                                hd->loc);
                        return NULL;
                }

                lx = sv_view(hd->lx);
                base = NULL;

                if (!strcmp(lx, TYPE_VOID))
                        base = (type *)type_void_alloc();
                else if (!strcmp(lx, TYPE_I32))
                        base = (type *)type_i32_alloc();
                else if (!strcmp(lx, TYPE_U8))
                        base = (type *)type_u8_alloc();
                else if (!strcmp(lx, TYPE_U32))
                        base = (type *)type_u32_alloc();
                else
                        fatal("unhandled type");
        }

 ptrs:
        while (lexer_peek(p->l)->k == TOKEN_KIND_ASTERISK) {
                lexer_discard(p->l);
                base = (type *)type_ptr_alloc(base, &p->a);
        }

        return base;
}

static stmt_expr *
parse_stmt_expr(parser *p)
{
        expr *e;

        if (!(e = parse_expr(p)))
                return NULL;

        if (!expect(p, TOKEN_KIND_SEMICOLON))
                return NULL;

        return stmt_expr_alloc(e, &p->a);
}

static stmt_block *
parse_stmt_block(parser *p)
{
        const token *begin;
        stmtp_ar stmts = array_empty();

        if (!(begin = expect(p, TOKEN_KIND_LCURLY)))
                return NULL;

        while (lexer_peek(p->l)->k != TOKEN_KIND_RCURLY
               && lexer_peek(p->l)->k != TOKEN_KIND_EOF) {
                stmt *s;
                if (!(s = parse_stmt(p)))
                        return NULL;
                array_append(stmts, s);
        }

        if (!expect(p, TOKEN_KIND_RCURLY))
                return NULL;

        return stmt_block_alloc(stmts, begin->loc, &p->a);
}

static stmt_let *
parse_stmt_let(parser *p)
{
        const token *begin;
        const token *id;
        type *ty;
        expr *e;

        if (!(begin = expect_keyword(p, KEYWORD_LET)))
                return NULL;

        if (!(id = expect(p, TOKEN_KIND_IDENTIFIER)))
                return NULL;

        if (!expect(p, TOKEN_KIND_COLON))
                return NULL;

        if (!(ty = parse_type(p)))
                return NULL;

        if (lexer_peek(p->l)->k == TOKEN_KIND_EQUALS) {
                lexer_discard(p->l); // =
                if (!(e = parse_expr(p)))
                        return NULL;
        }
        else
                e = NULL;

        if (!expect(p, TOKEN_KIND_SEMICOLON))
                return NULL;

        return stmt_let_alloc(id, ty, e, begin->loc, &p->a);
}

static int
parse_proc_parameters(parser        *p,
                      proc_param_ar *o_params,
                      uint32_t      *o_flags)
{
        *o_params = array_empty_explicit(proc_param_ar);

        if (!expect(p, TOKEN_KIND_LPAREN))
                return 0;

        while (lexer_peek(p->l)->k != TOKEN_KIND_RPAREN
               && lexer_peek(p->l)->k != TOKEN_KIND_EOF) {
                const token *id;
                type *ty;

                if (lexer_peek(p->l)->k == TOKEN_KIND_ELLIPSIS) {
                        lexer_discard(p->l); // ...
                        *o_flags |= PROC_FLAG_VARIADIC;
                        if (!expect(p, TOKEN_KIND_RPAREN))
                                return 0;
                        goto ok;
                }

                if (!(id = expect(p, TOKEN_KIND_IDENTIFIER)))
                        return 0;
                if (!expect(p, TOKEN_KIND_COLON))
                        return 0;
                if (!(ty = parse_type(p)))
                        return 0;

                array_append(*o_params, ((proc_param) {
                        .id = id,
                        .ty = ty,
                        .bind = NULL,
                }));

                if (lexer_peek(p->l)->k != TOKEN_KIND_COMMA)
                        break;

                lexer_discard(p->l); // ,
        }

        if (!expect(p, TOKEN_KIND_RPAREN))
                return 0;
ok:
        return 1;
}

static stmt_proc *
parse_stmt_proc(parser *p)
{
        const token *begin;
        const char *beginlx;
        uint32_t flags;
        const token *id;
        proc_param_ar params = array_empty();
        type *ty;
        stmt_block *body;

        begin = NULL;
        beginlx = sv_view(lexer_peek(p->l)->lx);
        flags = 0x00000000;

        if (!strcmp(beginlx, KEYWORD_EXPORT)) {
                begin = lexer_next(p->l);
                flags |= PROC_FLAG_EXPORT;
        }

        if (!strcmp(beginlx, KEYWORD_EXTERN)) {
                begin = lexer_next(p->l);
                flags |= PROC_FLAG_EXTERN;
        }

        if (!begin) {
                if (!(begin = expect_keyword(p, KEYWORD_PROC)))
                        return NULL;
        } else {
                if (!expect_keyword(p, KEYWORD_PROC))
                        return NULL;
        }

        if (!(id = expect(p, TOKEN_KIND_IDENTIFIER)))
                return NULL;

        if (!parse_proc_parameters(p, &params, &flags))
                return NULL;

        if (!expect(p, TOKEN_KIND_COLON))
                return NULL;

        if (!(ty = parse_type(p)))
                return NULL;

        if ((flags & PROC_FLAG_EXTERN) == 0) {
                if (!(body = parse_stmt_block(p)))
                        return NULL;
        } else {
                if (!expect(p, TOKEN_KIND_SEMICOLON))
                        return NULL;
                body = NULL;
        }

        return stmt_proc_alloc(flags, id, params, ty, body, begin->loc, &p->a);
}

static stmt_if *
parse_stmt_if(parser *p)
{
        const token *begin;
        expr *e;
        stmt *then;
        stmt *else_;

        if (!(begin = expect_keyword(p, KEYWORD_IF)))
                return NULL;

        if (!(e = parse_expr(p)))
                return NULL;

        if (!(then = parse_stmt(p)))
                return NULL;

        if (lexer_peek(p->l)->k == TOKEN_KIND_KEYWORD
            && !strcmp(sv_view(lexer_peek(p->l)->lx), KEYWORD_ELSE)) {
                lexer_discard(p->l); // else
                if (!(else_ = parse_stmt(p)))
                        return NULL;
        }
        else
                else_ = NULL;

        return stmt_if_alloc(e, then, else_, begin->loc, &p->a);
}

static stmt_while *
parse_stmt_while(parser *p)
{
        const token *begin;
        expr *e;
        stmt *body;

        if (!(begin = expect_keyword(p, KEYWORD_WHILE)))
               return NULL;

       if (!(e = parse_expr(p)))
               return NULL;

       if (!(body = parse_stmt(p)))
               return NULL;

        return stmt_while_alloc(e, body, begin->loc, &p->a);
}

static stmt_return *
parse_stmt_return(parser *p)
{
        const token *begin;
        expr *e;

        if (!(begin = expect_keyword(p, KEYWORD_RETURN)))
                return NULL;

        if (lexer_peek(p->l)->k != TOKEN_KIND_SEMICOLON) {
                if (!(e = parse_expr(p)))
                        return NULL;
        }
        else
                e = NULL;

        if (!expect(p, TOKEN_KIND_SEMICOLON))
                return NULL;

        return stmt_return_alloc(e, begin->loc, &p->a);
}

static int
parse_struct_members(parser           *p,
                     struct_member_ar *o_members)
{
        *o_members = array_empty_explicit(struct_member_ar);

        if (!expect(p, TOKEN_KIND_LCURLY))
                return 0;

        while (lexer_peek(p->l)->k != TOKEN_KIND_RCURLY
               && lexer_peek(p->l)->k != TOKEN_KIND_EOF) {
                const token *id;
                type *ty;

                if (!(id = expect(p, TOKEN_KIND_IDENTIFIER)))
                        return 0;
                if (!expect(p, TOKEN_KIND_COLON))
                        return 0;
                if (!(ty = parse_type(p)))
                        return 0;

                array_append(*o_members, ((struct_member) {
                        .id = id,
                        .ty = ty,
                        .bind = NULL,
                }));

                if (lexer_peek(p->l)->k != TOKEN_KIND_COMMA)
                        break;

                lexer_discard(p->l); // ,
        }

        if (!expect(p, TOKEN_KIND_RCURLY))
                return 0;

        return 1;
}

static stmt_struct *
parse_stmt_struct(parser *p)
{
        const token *begin;
        const token *id;
        struct_member_ar members;

        if (!(begin = expect_keyword(p, KEYWORD_STRUCT)))
                return NULL;

        if (!(id = expect(p, TOKEN_KIND_IDENTIFIER)))
                return NULL;

        if (!parse_struct_members(p, &members))
                return NULL;

        return stmt_struct_alloc(id, members, begin->loc, &p->a);
}

static stmt_exit *
parse_stmt_exit(parser *p)
{
        const token *begin;
        expr *e;

        if (!(begin = expect_keyword(p, KEYWORD_EXIT)))
                return NULL;

        if (!(e = parse_expr(p)))
                return NULL;

        if (!expect(p, TOKEN_KIND_SEMICOLON))
                return NULL;

        return stmt_exit_alloc(e, begin->loc, &p->a);
}

static stmt *
parse_stmt_keyword(parser *p)
{
        const token *hd;
        const char *lx;

        hd = lexer_peek(p->l);
        lx = sv_view(hd->lx);

        if (!strcmp(lx, KEYWORD_LET))
                return (stmt *)parse_stmt_let(p);
        if (!strcmp(lx, KEYWORD_PROC)
            || !strcmp(lx, KEYWORD_EXPORT)
            || !strcmp(lx, KEYWORD_EXTERN))
                return (stmt *)parse_stmt_proc(p);
        if (!strcmp(lx, KEYWORD_RETURN))
                return (stmt *)parse_stmt_return(p);
        if (!strcmp(lx, KEYWORD_IF))
                return (stmt *)parse_stmt_if(p);
        if (!strcmp(lx, KEYWORD_WHILE))
                return (stmt *)parse_stmt_while(p);
        if (!strcmp(lx, KEYWORD_STRUCT))
                return (stmt *)parse_stmt_struct(p);
        if (!strcmp(lx, KEYWORD_EXIT))
                return (stmt *)parse_stmt_exit(p);

        p->error = comperr_from(str_from_fmt("illegal keyword `%s'", lx), hd->loc);
        return NULL;
}

static stmt *
parse_stmt(parser *p)
{
        const token *hd;

        hd = lexer_peek(p->l);

        if (hd->k == TOKEN_KIND_KEYWORD)
                return parse_stmt_keyword(p);
        if (hd->k == TOKEN_KIND_LCURLY)
                return (stmt *)parse_stmt_block(p);
        return (stmt *)parse_stmt_expr(p);
}

parser
parse(MOVE(lexer *l))
{
        parser p;

        p = (parser) {
                .l = l,
                .stmts = array_empty(),
                .error = (comperr) {0},
                .a = (arena) {0},
        };

        arena_init(&p.a, ARENA_DEFAULT_BLK_SIZE);

        while (lexer_peek(p.l)->k != TOKEN_KIND_EOF) {
                stmt *s;

                if (!(s = parse_stmt(&p)))
                        break;

                array_append(p.stmts, s);
        }

        return p;
}
