#include "lexer.h"
#include "io.h"

#include <stdio.h>

int
main(void)
{
        init_lexer_interface();

        const char *path = "1.in";
        lexer l = lex_file(path, load_file(path));

        (void)l;

        return 0;
}
