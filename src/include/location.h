#ifndef LOCATION_H_INCLUDED
#define LOCATION_H_INCLUDED

#include <stddef.h>

typedef struct {
        const char *path;
        size_t r;
        size_t c;
} location;

location location_from(const char *path, size_t r, size_t c);
const char *location_fmt(location l);

#endif // LOCATION_H_INCLUDED
