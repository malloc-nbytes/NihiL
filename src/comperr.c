#include "comperr.h"

comperr
comperr_from(str msg, location loc)
{
        return (comperr) {
                .msg = msg,
                .loc = loc,
        };
}
