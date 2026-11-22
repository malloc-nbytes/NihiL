#ifndef SEMANTIC_H_INCLUDED
#define SEMANTIC_H_INCLUDED

#include "array.h"
#include "comperr.h"
#include "parser.h"
#include "types.h"
#include "scope.h"
#include "mem.h"

typedef enum {
        BINDING_KIND_VARIABLE = 0,
        BINDING_KIND_PROCEDURE,
        BINDING_KIND_PARAMETER,
        BINDING_KIND_TYPE,
} binding_kind;

typedef struct binding {
        binding_kind k;
        const token *id;
        type *ty;
        void *declaration;
} binding;

ARRAY_DEFINE(binding *, bindingp_ar);

typedef struct {
        parser *p;
        comperr_ar errors;
        bindingp_ar bindings;
        scope *global;
        scope *current;
        type *expected_type;
        arena a;
} semantic_checker;

semantic_checker semantic_analysis(MOVE(parser *p));

#endif // SEMANTIC_H_INCLUDED
