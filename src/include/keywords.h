#ifndef KEYWORDS_H_INCLUDED
#define KEYWORDS_H_INCLUDED

#include <stddef.h>

#define KEYWORD_IF     "if"
#define KEYWORD_ELSE   "else"
#define KEYWORD_PROC   "proc"
#define KEYWORD_EXPORT "export"
#define KEYWORD_EXTERN "extern"
#define KEYWORD_LET    "let"
#define KEYWORD_WHILE  "while"
#define KEYWORD_RETURN "return"

#define KEYWORDS_CPL { \
        KEYWORD_IF, \
        KEYWORD_ELSE, \
        KEYWORD_PROC, \
        KEYWORD_EXPORT, \
        KEYWORD_EXTERN, \
        KEYWORD_LET, \
        KEYWORD_WHILE, \
        KEYWORD_RETURN, \
        NULL, \
}

int is_keyword(const char *s);

#endif // KEYWORDS_H_INCLUDED
