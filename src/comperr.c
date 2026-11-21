#include "comperr.h"

#include <stdio.h>

comperr
comperr_from(str msg, location loc)
{
        return (comperr) {
                .msg = msg,
                .loc = loc,
                .set = 1,
        };
}

const char *
comperr_format(comperr err)
{
        static char buf[512] = {0};

        sprintf(buf, "%s: %s", location_fmt(err.loc), err.msg.chars);

        return buf;
}
