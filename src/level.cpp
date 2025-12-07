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

std::vector<MeshTriangle> get_level_tris(Level *level){
    std::vector<MeshTriangle> worldTris;
    worldTris.reserve(500000); // optional

    for(auto& shape : level->shapes){
        auto& verts = shape.mesh.vertices; // vetor de float
        auto& inds  = shape.mesh.indices;  // vetor de uint32

        for (int i = 0; i < inds.size(); i += 3){
            uint32_t i0 = inds[i];
            uint32_t i1 = inds[i+1];
            uint32_t i2 = inds[i+2];

            // cada vert tem 8 floats
            int v0 = i0 * 8;
            int v1 = i1 * 8;
            int v2 = i2 * 8;

            MeshTriangle t;
            t.a = glm::vec3(verts[v0], verts[v0+1], verts[v0+2]);
            t.b = glm::vec3(verts[v1], verts[v1+1], verts[v1+2]);
            t.c = glm::vec3(verts[v2], verts[v2+1], verts[v2+2]);

            // opcional: aplicar transform do node do gltf
            // t.a = mat * vec4(t.a, 1);
            // t.b = mat * vec4(t.b, 1);
            // t.c = mat * vec4(t.c, 1);

            worldTris.push_back(t);
        }
    }

    return worldTris;
}
