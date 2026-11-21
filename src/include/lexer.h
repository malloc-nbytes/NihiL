#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include "arena.h"
#include "location.h"
#include "mem.h"
#include "array.h"
#include "sv.h"

typedef enum {
        TOKEN_KIND_EOF = 0,
        TOKEN_KIND_IDENTIFIER,
        TOKEN_KIND_KEYWORD,
        TOKEN_KIND_TYPE,
        TOKEN_KIND_INTEGER_LITERAL,
        TOKEN_KIND_STRING_LITERAL,
        TOKEN_KIND_CHARACTER_LITERAL,
        TOKEN_KIND_LPAREN,
        TOKEN_KIND_RPAREN,
        TOKEN_KIND_LSQR,
        TOKEN_KIND_RSQR,
        TOKEN_KIND_LCURLY,
        TOKEN_KIND_RCURLY,
        TOKEN_KIND_SEMICOLON,
        TOKEN_KIND_COMMA,
        TOKEN_KIND_PLUS,
        TOKEN_KIND_MINUS,
        TOKEN_KIND_ASTERISK,
        TOKEN_KIND_FSLASH,
        TOKEN_KIND_PERCENT,
        TOKEN_KIND_AMPERSAND,
        TOKEN_KIND_PIPE,
        TOKEN_KIND_UPTICK,
        TOKEN_KIND_BANG,
        TOKEN_KIND_LESSTHAN,
        TOKEN_KIND_GREATERTHAN,
        TOKEN_KIND_EQUALS,
        TOKEN_KIND_COLON,
        TOKEN_KIND_QUESTION,
        TOKEN_KIND_PERIOD,
        TOKEN_KIND_TILDE,
        TOKEN_KIND_DOUBLE_EQUALS,
        TOKEN_KIND_BANG_EQUALS,
        TOKEN_KIND_GREATERTHAN_EQUALS,
        TOKEN_KIND_LESSTHAN_EQUALS,
        TOKEN_KIND_TILDE_EQUALS,
        TOKEN_KIND_PIPE_EQUALS,
        TOKEN_KIND_AMPERSAND_EQUALS,
        TOKEN_KIND_PLUS_EQUALS,
        TOKEN_KIND_MINUS_EQUALS,
        TOKEN_KIND_ASTERISK_EQUALS,
        TOKEN_KIND_FSLASH_EQUALS,
        TOKEN_KIND_PERCENT_EQUALS,
        TOKEN_KIND_DOUBLE_PIPE,
        TOKEN_KIND_DOUBLE_AMPERSAND,
        TOKEN_KIND_ELLIPSIS,

        __TOKEN_KIND_LIMIT,
} token_kind;

typedef struct {
        sv lx;
        token_kind k;
        location loc;
} token;

ARRAY_DEFINE(token *, tokenp_ar);

typedef struct {
        tokenp_ar ts;
        char *src;
        char *path;
        size_t cursor;
        arena a;
} lexer;

void init_lexer_translation_unit(void);
const char *token_kind_to_ccstr(token_kind k);
lexer lex_file(const char *path, MOVE(char *src));
void lexer_dump(lexer l);
const token *lexer_peek(const lexer *l);
token *lexer_next(lexer *l);
void lexer_discard(lexer *l);

#endif // LEXER_H_INCLUDED
