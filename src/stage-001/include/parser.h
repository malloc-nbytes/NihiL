#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "grammar.h"
#include "arena.h"
#include "comperr.h"
#include "mem.h"

typedef struct {
        lexer *l;
        stmtp_ar stmts;
        comperr error;
        arena a;
} parser;

parser parse(MOVE(lexer) *l);

#endif // PARSER_H_INCLUDED
