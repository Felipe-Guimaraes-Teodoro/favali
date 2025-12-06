#include "level.h"

#include <stdlib.h>

Level *create_level() {
    Level *l = (Level*) calloc(1, sizeof(Level));

    return l;
}