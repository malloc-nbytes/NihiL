#include "parser.h"
#include "grammar.h"
#include "lexer.h"
#include "keywords.h"
#include "error.h"

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

static expr *
parse_primary_expr(parser *p)
{
        const token *hd = lexer_peek(p->l);

        switch (hd->k) {
        case TOKEN_KIND_INTEGER_LITERAL:
                lexer_discard(p->l);
                return (expr *)expr_integer_alloc(hd, &p->a);
        case TOKEN_KIND_IDENTIFIER:
                lexer_discard(p->l);
                return (expr *)expr_identifier_alloc(hd, &p->a);
        case TOKEN_KIND_STRING_LITERAL:
                lexer_discard(p->l);
                return (expr *)expr_string_alloc(hd, &p->a);
        default:
                break;
        }

        return NULL;
}

static expr *
parse_unary_expr(parser *p)
{
        // TODO
        return parse_primary_expr(p);
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
                token *op = lexer_next(p->l);
                expr *rhs = parse_unary_expr(p);
                expr_binary *bin = expr_binary_alloc(lhs, op, rhs, &p->a);
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
                token *op = lexer_next(p->l);
                expr *rhs = parse_multiplicative_expr(p);
                expr_binary *bin = expr_binary_alloc(lhs, op, rhs, &p->a);
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
                token *op = lexer_next(p->l);
                expr *rhs = parse_additive_expr(p);
                expr_binary *bin = expr_binary_alloc(lhs, op, rhs, &p->a);
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
                token *op = lexer_next(p->l);
                expr *rhs = parse_equalitive_expr(p);
                expr_binary *bin = expr_binary_alloc(lhs, op, rhs, &p->a);
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
                token *op = lexer_next(p->l);
                expr *rhs = parse_assignment_expr(p);
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
        token *hd;
        const char *lx;
        type *base;

        hd = expect(p, TOKEN_KIND_TYPE);
        lx = sv_view(hd->lx);
        base = NULL;

        if (!strcmp(lx, TYPE_VOID))
                base = (type *)type_void_alloc();
        else if (!strcmp(lx, TYPE_I32))
                base = (type *)type_i32_alloc();
        else if (!strcmp(lx, TYPE_U8))
                base = (type *)type_u8_alloc();
        else
                fatal("unhandled type");

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
                if (!(ty = parse_type(p)))
                        return 0;

                if (lexer_peek(p->l)->k == TOKEN_KIND_COMMA)
                        lexer_discard(p->l); // ,
                else
                        break;
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
        id = NULL;
        ty = NULL;
        body = NULL;

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

        if (!(ty = parse_type(p)))
                return NULL;

        if (!(body = parse_stmt_block(p)))
                return NULL;

        return stmt_proc_alloc(flags, id, params, ty, body, begin->loc, &p->a);
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
        if (!strcmp(lx, KEYWORD_PROC))
                return (stmt *)parse_stmt_proc(p);

        fatal("unknown keyword: `%s'\n", lx);
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
