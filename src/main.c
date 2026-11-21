#include "lexer.h"
#include "io.h"

int
main(void)
{
        init_lexer_translation_unit();
        lexer l = lex_file("1.in", load_file("1.in"));
        lexer_dump(l);

        return 0;
}
