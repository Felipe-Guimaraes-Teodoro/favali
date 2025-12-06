#include "level.h"

#include <stdlib.h>

#define VERTEX_STRIDE 8

Level *create_level() {
    Level *l = (Level*) calloc(1, sizeof(Level));

    return l;
}

