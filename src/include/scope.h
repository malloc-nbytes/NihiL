#ifndef SCOPE_H_INCLUDED
#define SCOPE_H_INCLUDED

#include "arena.h"
#include "array.h"

typedef struct scope scope;

typedef struct {
        char *name;
        void *value;
} scope_entry;

ARRAY_DEFINE(scope_entry, scope_entry_ar);

struct scope {
        scope *parent;
        scope_entry_ar entries;
};


/*
 * Values must be non-NULL pointers owned by the caller.  A scope owns its
 * entry names, but never its values or its parent.
 */
scope *scope_create(scope *parent, arena *a);
void scope_destroy(scope *scope);

/*
 * Adds a name only to this scope.  Returns false if name is invalid or is
 * already declared in this scope.
 */
int scope_bind(scope *scope, const char *name, void *value, arena *a);

void *scope_lookup_local(const scope *scope, const char *name);
void *scope_lookup(const scope *scope, const char *name);

#endif // SCOPE_H_INCLUDED

