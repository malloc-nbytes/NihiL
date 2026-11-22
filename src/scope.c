#include "scope.h"
#include "mem.h"

#include <stdlib.h>
#include <string.h>

static char *
scope_name_copy(const char *name, arena *a)
{
        size_t len;
        char *copy;

        len = strlen(name);
        copy = (char *)arena_alloc(a, len+1);
        memcpy(copy, name, len+1);

        return copy;
}

scope *
scope_create(scope *parent, arena *a)
{
        scope *result;

        result = (scope *)arena_alloc(a, sizeof(*result));
        result->parent = parent;
        result->entries = array_empty_explicit(scope_entry_ar);

        return result;
}

void
scope_destroy(scope *sc)
{
        if (!sc)
                return;

        array_free(sc->entries);
}

void *
scope_lookup_local(const scope *sc,
                   const char  *name)
{
        if (!sc || !name)
                return NULL;

        for (size_t i = 0; i < sc->entries.len; ++i) {
                scope_entry *entry;

                entry = &sc->entries.data[i];
                if (!strcmp(entry->name, name))
                        return entry->value;
        }

        return NULL;
}

void *
scope_lookup(const scope *sc,
             const char  *name)
{
        for (; sc; sc = sc->parent) {
                void *value;

                value = scope_lookup_local(sc, name);
                if (value)
                        return value;
        }

        return NULL;
}

int
scope_bind(scope      *sc,
           const char *name,
           void       *value,
           arena      *a)
{
        if (!sc || !name || !value || scope_lookup_local(sc, name))
                return 0;

        array_append(sc->entries, ((scope_entry) {
                .name = scope_name_copy(name, a),
                .value = value,
        }));

        return 1;
}

