#include "keywords.h"

#include <string.h>

int
is_keyword(const char *s)
{
        static const char *kwds[] = KEYWORDS_CPL;

        for (size_t i = 0; kwds[i]; ++i) {
                if (!strcmp(s, kwds[i]))
                        return 1;
        }

        return 0;
}
