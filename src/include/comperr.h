#ifndef COMPERR_H_INCLUDED
#define COMPERR_H_INCLUDED

#include "str.h"
#include "array.h"
#include "location.h"

typedef struct {
        str msg;
        location loc;
        int set;
} comperr;

ARRAY_DEFINE(comperr, comperr_ar);

comperr comperr_from(str msg, location loc);

#endif // COMPERR_H_INCLUDED
