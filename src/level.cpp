#include "level.h"

#include <stdlib.h>

#define VERTEX_STRIDE 8

Level *create_level() {
    Level *l = (Level*) calloc(1, sizeof(Level));

    return l;
}

/// Merge all static shapes into one big static shape
/// Reduces draw calls but also reduces culling opportunities
void merge_level_shapes(Level* level) {
    unsigned int base_index = 0;

    Shape god_shape = make_shape(Shapes::Empty);

    for (Shape& shape : level->shapes) {
        god_shape.mesh.vertices.insert(
            god_shape.mesh.vertices.end(),
            shape.mesh.vertices.begin(),
            shape.mesh.vertices.end()
        );

        for (unsigned int idx : shape.mesh.indices) {
            god_shape.mesh.indices.push_back(idx + base_index);
        }

        base_index += shape.mesh.vertices.size() / VERTEX_STRIDE;

        // shape.mesh.clear_resources(); unimplemented!
    }

    level->shapes.clear(); // memory leak: ogl resources arent cleared yet

    setup_mesh(god_shape.mesh);
    level->shapes.push_back(std::move(god_shape));
}

