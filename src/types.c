#include "types.h"

#include <stddef.h>
#include <string.h>

int
is_type(const char *s)
{
        static const char *t[] = TYPES_CPL;

        for (size_t i = 0; i < sizeof(t)/sizeof(*t); ++i)
                if (!strcmp(s, t[i]))
                        return 1;

        return 0;
}
