#include "lexer.h"
#include "parser.h"
#include "semantic.h"
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

        lexer l = lex_file("1.nl", load_file("1.nl"));
        parser p = parse(MOVE(&l));

        if (p.error.set) {
                fprintf(stderr, "%s\n", comperr_format(p.error));
                return 1;
        }

        semantic_checker checker = semantic_analysis(MOVE(&p));

        if (checker.errors.len) {
                for (size_t i = 0; i < checker.errors.len; ++i)
                        fprintf(stderr, "%s\n",
                                comperr_format(checker.errors.data[i]));
                return 1;
        }

        return 0;
}
