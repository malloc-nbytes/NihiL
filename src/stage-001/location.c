#include "location.h"

#include <stdio.h>
#include <string.h>

location
location_from(const char *path, size_t r, size_t c)
{
        return (location) {
                .path = path,
                .r = r,
                .c = c,
        };
}

const char *
location_fmt(location l)
{
        static char buf[1024];
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%s:%zu:%zu", l.path, l.r, l.c);
        return buf;
}
