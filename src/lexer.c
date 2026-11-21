#include "lexer.h"
#include "arena.h"
#include "array.h"
#include "keywords.h"
#include "types.h"
#include "location.h"
#include "sv.h"
#include "map.h"
#include "error.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

MAP_DEFINE(const char *, token_kind, opmap);
MAP_IMPL(const char *, token_kind, opmap);

static opmap g_opmap = {0};

static unsigned
opmap_hash(const char **s)
{
        return (unsigned)**s;
}

static int
opmap_cmp(const char **s0,
          const char **s1)
{
        return strcmp(*s0, *s1);
}

void
init_lexer_translation_unit(void)
{
        g_opmap = opmap_create(opmap_hash, opmap_cmp);
        opmap_insert(&g_opmap, "(", TOKEN_KIND_LPAREN);
        opmap_insert(&g_opmap, ")", TOKEN_KIND_RPAREN);
        opmap_insert(&g_opmap, "[", TOKEN_KIND_LSQR);
        opmap_insert(&g_opmap, "]", TOKEN_KIND_RSQR);
        opmap_insert(&g_opmap, "{", TOKEN_KIND_LCURLY);
        opmap_insert(&g_opmap, "}", TOKEN_KIND_RCURLY);
        opmap_insert(&g_opmap, ";", TOKEN_KIND_SEMICOLON);
        opmap_insert(&g_opmap, ",", TOKEN_KIND_COMMA);
        opmap_insert(&g_opmap, "+", TOKEN_KIND_PLUS);
        opmap_insert(&g_opmap, "-", TOKEN_KIND_MINUS);
        opmap_insert(&g_opmap, "*", TOKEN_KIND_ASTERISK);
        opmap_insert(&g_opmap, "/", TOKEN_KIND_FSLASH);
        opmap_insert(&g_opmap, "%", TOKEN_KIND_PERCENT);
        opmap_insert(&g_opmap, "&", TOKEN_KIND_AMPERSAND);
        opmap_insert(&g_opmap, "|", TOKEN_KIND_PIPE);
        opmap_insert(&g_opmap, "^", TOKEN_KIND_UPTICK);
        opmap_insert(&g_opmap, "!", TOKEN_KIND_BANG);
        opmap_insert(&g_opmap, "<", TOKEN_KIND_LESSTHAN);
        opmap_insert(&g_opmap, ">", TOKEN_KIND_GREATERTHAN);
        opmap_insert(&g_opmap, "=", TOKEN_KIND_EQUALS);
        opmap_insert(&g_opmap, ":", TOKEN_KIND_COLON);
        opmap_insert(&g_opmap, "?", TOKEN_KIND_QUESTION);
        opmap_insert(&g_opmap, ".", TOKEN_KIND_PERIOD);
        opmap_insert(&g_opmap, "~", TOKEN_KIND_TILDE);
        opmap_insert(&g_opmap, "==", TOKEN_KIND_DOUBLE_EQUALS);
        opmap_insert(&g_opmap, "!=", TOKEN_KIND_BANG_EQUALS);
        opmap_insert(&g_opmap, ">=", TOKEN_KIND_GREATERTHAN_EQUALS);
        opmap_insert(&g_opmap, "<=", TOKEN_KIND_LESSTHAN_EQUALS);
        opmap_insert(&g_opmap, "~=", TOKEN_KIND_TILDE_EQUALS);
        opmap_insert(&g_opmap, "|=", TOKEN_KIND_PIPE_EQUALS);
        opmap_insert(&g_opmap, "&=", TOKEN_KIND_AMPERSAND_EQUALS);
        opmap_insert(&g_opmap, "+=", TOKEN_KIND_PLUS_EQUALS);
        opmap_insert(&g_opmap, "-=", TOKEN_KIND_MINUS_EQUALS);
        opmap_insert(&g_opmap, "*=", TOKEN_KIND_ASTERISK_EQUALS);
        opmap_insert(&g_opmap, "/=", TOKEN_KIND_FSLASH_EQUALS);
        opmap_insert(&g_opmap, "%=", TOKEN_KIND_PERCENT_EQUALS);
        opmap_insert(&g_opmap, "||", TOKEN_KIND_DOUBLE_PIPE);
        opmap_insert(&g_opmap, "&&", TOKEN_KIND_DOUBLE_AMPERSAND);
        opmap_insert(&g_opmap, "...", TOKEN_KIND_ELLIPSIS);
}

const char *
token_kind_to_ccstr(token_kind k)
{
        switch (k) {
        case TOKEN_KIND_EOF: return "EOF";
        case TOKEN_KIND_IDENTIFIER: return "identifier";
        case TOKEN_KIND_KEYWORD: return "keyword";
        case TOKEN_KIND_TYPE: return "type";
        case TOKEN_KIND_INTEGER_LITERAL: return "integer";
        case TOKEN_KIND_STRING_LITERAL: return "string";
        case TOKEN_KIND_CHARACTER_LITERAL: return "character";
        case TOKEN_KIND_LPAREN: return "(";
        case TOKEN_KIND_RPAREN: return ")";
        case TOKEN_KIND_LSQR: return "[";
        case TOKEN_KIND_RSQR: return "]";
        case TOKEN_KIND_LCURLY: return "{";
        case TOKEN_KIND_RCURLY: return "}";
        case TOKEN_KIND_SEMICOLON: return ";";
        case TOKEN_KIND_COMMA: return ",";
        case TOKEN_KIND_PLUS: return "+";
        case TOKEN_KIND_MINUS: return "-";
        case TOKEN_KIND_ASTERISK: return "*";
        case TOKEN_KIND_FSLASH: return "/";
        case TOKEN_KIND_PERCENT: return "%";
        case TOKEN_KIND_AMPERSAND: return "&";
        case TOKEN_KIND_PIPE: return "|";
        case TOKEN_KIND_UPTICK: return "^";
        case TOKEN_KIND_BANG: return "!";
        case TOKEN_KIND_LESSTHAN: return "<";
        case TOKEN_KIND_GREATERTHAN: return ">";
        case TOKEN_KIND_EQUALS: return "=";
        case TOKEN_KIND_COLON: return ":";
        case TOKEN_KIND_QUESTION: return "?";
        case TOKEN_KIND_PERIOD: return ".";
        case TOKEN_KIND_TILDE: return "~";
        case TOKEN_KIND_DOUBLE_EQUALS: return "==";
        case TOKEN_KIND_BANG_EQUALS: return "!=";
        case TOKEN_KIND_GREATERTHAN_EQUALS: return ">=";
        case TOKEN_KIND_LESSTHAN_EQUALS: return "<=";
        case TOKEN_KIND_TILDE_EQUALS: return "~=";
        case TOKEN_KIND_PIPE_EQUALS: return "|=";
        case TOKEN_KIND_AMPERSAND_EQUALS: return "&=";
        case TOKEN_KIND_PLUS_EQUALS: return "+=";
        case TOKEN_KIND_MINUS_EQUALS: return "-=";
        case TOKEN_KIND_ASTERISK_EQUALS: return "*=";
        case TOKEN_KIND_FSLASH_EQUALS: return "/=";
        case TOKEN_KIND_PERCENT_EQUALS: return "%=";
        case TOKEN_KIND_DOUBLE_PIPE: return "||";
        case TOKEN_KIND_DOUBLE_AMPERSAND: return "&&";
        case TOKEN_KIND_ELLIPSIS: return "...";
        default: return "<UNKNOWN>";
        }
        // unreachable
}

static token *
token_alloc(const char *s,
            size_t      n,
            token_kind  k,
            const char *path,
            size_t      r,
            size_t      c,
            arena      *a)
{
        token *t;

        t = (token *)arena_alloc(a, sizeof(*t));
        t->lx = sv_from(s, (ssize_t)n);
        t->k = k;
        t->loc = location_from(path, r, c);

        return t;
}

static size_t
consume_while(const char *s, int (*pred)(int))
{
        size_t i;
        for (i = 0; s[i] && pred(s[i]); ++i);
        return i;
}

static inline int
isident(int c)
{
        return isalnum(c) || c == '_';
}

static inline int
notquote(int c)
{
        return c != '"';
}

static inline int
notsquote(int c)
{
        return c != '\'';
}

static inline int
issym(int c)
{
        return !isident(c)
                && notquote(c)
                && notsquote(c)
                && !isspace(c);
}

static token_kind *
determine_sym(const char *s, size_t *len)
{
        token_kind *k;
        char buf[512] = {0};

        if (*len >= sizeof(buf))
                *len = sizeof(buf)-1;

        k = NULL;
        memcpy(buf, s, *len);

        while (*len) {
                if (opmap_contains(&g_opmap, buf)) {
                        k = opmap_get(&g_opmap, buf);
                        break;
                }

                buf[--(*len)] = 0;
        }

        return k;
}

const token *
lexer_peek(const lexer *l)
{
        if (l->cursor >= l->ts.len)
                return l->ts.data[l->ts.len-1];
        return l->ts.data[l->cursor];
}

token *
lexer_next(lexer *l)
{
        if (l->cursor >= l->ts.len)
                return l->ts.data[l->ts.len-1];
        return l->ts.data[l->cursor++];
}

void
lexer_discard(lexer *l)
{
        if (l->cursor < l->ts.len)
                ++l->cursor;
}

void
lexer_dump(lexer l)
{
        for (size_t i = 0; i < l.ts.len; ++i) {
                const token *t = l.ts.data[i];
                printf("<lx=%s, k=%s, ", sv_view(t->lx), token_kind_to_ccstr(t->k));
                printf("loc=%s>\n", location_fmt(t->loc));
        }
}

lexer
lex_file(const char *path, MOVE(char *src))
{
        lexer l;
        size_t n, i, r, c;

        l = (lexer) {
                .ts = array_empty(),
                .src = src,
                .path = NULL,
                .cursor = 0,
                .a = (arena) {0},
        };

        n = strlen(src);
        i = 0;
        r = 1;
        c = 1;

        arena_init(&l.a, ARENA_DEFAULT_BLK_SIZE);
        l.path = (char *)arena_alloc(&l.a, n+1);
        strcpy(l.path, path);

        while (i < n) {
                char ch = l.src[i];

                if (ch == '\n') {
                        ++r;
                        c = 1;
                        ++i;
                } else if (isspace(ch)) {
                        ++c;
                        ++i;
                } else if (isalpha(ch) || ch == '_') {
                        size_t len = consume_while(l.src+i, isident);
                        token *t = token_alloc(l.src+i, len, TOKEN_KIND_IDENTIFIER, l.path, r, c, &l.a);
                        if (is_keyword(sv_view(t->lx)))
                                t->k = TOKEN_KIND_KEYWORD;
                        else if (is_type(sv_view(t->lx)))
                                t->k = TOKEN_KIND_TYPE;
                        array_append(l.ts, t);
                        i += len;
                        c += len;
                } else if (isdigit(ch)) {
                        size_t len = consume_while(l.src+i, isdigit);
                        token *t = token_alloc(l.src+i, len, TOKEN_KIND_INTEGER_LITERAL, l.path, r, c, &l.a);
                        array_append(l.ts, t);
                        i += len;
                        c += len;
                } else if (ch == '"') {
                        size_t len = consume_while(l.src+i+1, notquote);
                        token *t = token_alloc(l.src+i+1, len, TOKEN_KIND_STRING_LITERAL, l.path, r, c, &l.a);
                        array_append(l.ts, t);
                        i += len+2;
                        c += len+2;
                } else if (ch == '\'') {
                        assert(0 && "unimplemented");
                } else {
                        size_t len = consume_while(l.src+i, issym);
                        token_kind *k = determine_sym(l.src+i, &len);
                        if (!k || !len)
                                fatal("unknown operator at: `%c'", ch);
                        token *t = token_alloc(l.src+i, len, *k, l.path, r, c, &l.a);
                        array_append(l.ts, t);
                        i += len;
                        c += len;
                }
        }

        array_append(l.ts, token_alloc("EOF", 3, TOKEN_KIND_EOF, l.path, r, c, &l.a));

        return l;
}
