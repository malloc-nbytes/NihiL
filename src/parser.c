#include "parser.h"
#include "grammar.h"
#include "lexer.h"
#include "keywords.h"
#include "error.h"

#include <string.h>

static stmt *parse_stmt(parser *p);

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
parse_expr(parser *p)
{
        (void)p;
        return NULL;
}

static type *
parse_type(parser *p)
{
        (void)p;
        return NULL;
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

        while (lexer_peek(p->l)->k != TOKEN_KIND_RCURLY) {
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

static stmt_proc *
parse_stmt_proc(parser *p)
{
        (void)p;
        return NULL;
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
