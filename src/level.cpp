#include "level.h"

#include <stdlib.h>
#include <stdio.h>

#define VERTEX_STRIDE 8

Level *create_level() {
    Level *l = (Level*) calloc(1, sizeof(Level));

    return l;
}

Model *create_model() {
    Model *m = (Model*) calloc(1, sizeof(Model));

    return m;
}

void draw_level(Level *l, Camera& cam, unsigned int program, bool enable_culling) {
    // int i = 0;
    if (enable_culling) {
        for (const StaticShape& shape : l->shapes) {
            if (is_aabb_on_frustum(cam.frustum, shape.box)) {
                shape.draw(program, cam);
                // i++;
            }
        }
    } else {
        for (const StaticShape& shape : l->shapes) {
            shape.draw(program, cam);
        }
        // i++;
    }

    // printf("%u shapes avail, %u shapes drawn\n", l->shapes.size(),i);
}

void draw_model(Model *m, Camera& cam, unsigned int program) {
    // cam.update_frustum();
    for (const Shape& shape : m->shapes) {
        shape.draw(program, cam);
    } 
}

/// Merge all static shapes into one big static shape
/// Reduces draw calls but also reduces culling opportunities
/// Todo: merge only shapes that share the same material
/// aka: texture (since theres no actual material def)
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
    level->shapes.push_back(shape_to_static(std::move(god_shape)));
}

std::vector<MeshTriangle> get_level_tris(StaticShape *shape){
    std::vector<MeshTriangle> worldTris;
    worldTris.reserve(500000); // optional

    auto& verts = shape->mesh.vertices;
    auto& inds  = shape->mesh.indices;

    for (int i = 0; i < inds.size(); i += 3){
        uint32_t i0 = inds[i];
        uint32_t i1 = inds[i+1];
        uint32_t i2 = inds[i+2];

        // REMEMBER TO CHANGE THIS IF VERTEX STRUCTURE CHANGES!!!!
        // current is x, y, z, nx, ny, nz, uvx, uvz (total of 8)
        int v0 = i0 * 8;
        int v1 = i1 * 8;
        int v2 = i2 * 8;

        MeshTriangle t;
        t.a = glm::vec3(verts[v0], verts[v0+1], verts[v0+2]);
        t.b = glm::vec3(verts[v1], verts[v1+1], verts[v1+2]);
        t.c = glm::vec3(verts[v2], verts[v2+1], verts[v2+2]);

        // optional: apply gltf transform
        t.a = shape->transform.getModelMat() * glm::vec4(t.a, 1);
        t.b = shape->transform.getModelMat() * glm::vec4(t.b, 1);
        t.c = shape->transform.getModelMat() * glm::vec4(t.c, 1);

        worldTris.push_back(t);
    }

    return worldTris;
}
