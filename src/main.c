#include "lexer.h"
#include "parser.h"
#include "mem.h"
#include "types.h"
#include "io.h"

#include <stdio.h>

static void
init(void)
{
        init_lexer_translation_unit();
        init_types_translation_unit();
}

int
main(void)
{
        init();

        lexer l = lex_file("1.in", load_file("1.in"));
        parser p = parse(MOVE(&l));

        if (p.error.set) {
                fprintf(stderr, "%s\n", comperr_format(p.error));
                return 1;
        }

        return 0;
}
