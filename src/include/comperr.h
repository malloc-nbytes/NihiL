#ifndef COMPERR_H_INCLUDED
#define COMPERR_H_INCLUDED

#include "str.h"
#include "location.h"

typedef struct {
        str msg;
        location loc;
} comperr;

comperr comperr_from(str msg, location loc);

#endif // COMPERR_H_INCLUDED
