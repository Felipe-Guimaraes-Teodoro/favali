#include "gizmos.h"

#include "stdio.h"
#include "shaders.h"

#include <vector>
#include <array>
using std::vector;

const char* gizmo_vs = R"(
#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0f);

    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    TexCoord = aTexCoord;
}
)";

const char* gizmo_fs = R"(
#version 420 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform mat4 view;
uniform vec4 color;

void main() {
    FragColor = color;
}
)";

struct Gizmo {
    unsigned int mesh_idx;
    Transform transform;
    glm::vec4 color;
    int frames;
};

struct GizmoQueue {
    vector<Gizmo> gizmos;
                    /* as big as how many shapes were defined */
    std::array<Mesh, Shapes::Empty + 1> gizmo_meshes;

    unsigned int gizmo_program;
};

GizmoQueue *gizmo_queue = nullptr;

const std::array<Shapes, 3> supported_shapes = {
    Shapes::Cube,
    Shapes::Sphere,
    Shapes::Triangle
};

void init_vaos(GizmoQueue *queue) {
    for (Shapes shape : supported_shapes) {
        Shape tmp_shape = make_shape(shape);
        queue->gizmo_meshes[shape] = std::move(tmp_shape.mesh);
    }
}

void init_gizmos() {
    gizmo_queue = (GizmoQueue*) calloc(1, sizeof(GizmoQueue));

    init_vaos(gizmo_queue);

    Shader vs = create_shader(&gizmo_vs, GL_VERTEX_SHADER);
    Shader fs = create_shader(&gizmo_fs, GL_FRAGMENT_SHADER);
    gizmo_queue->gizmo_program = create_program(
        vs,
        fs
    );
}

void push_gizmo(Shapes shape, Transform t, glm::vec4 col) {
    gizmo_queue->gizmos.push_back( 
        (Gizmo) {
            .mesh_idx = shape,
            .transform = t,
            .color = col,
            .frames = 1,
        }
    );
}

void push_gizmo(Shapes shape, AABB aabb, glm::vec4 col) {
    glm::vec3 center = 0.5f * (aabb.min + aabb.max);
    glm::vec3 size = aabb.max - aabb.min;

    Transform t(center, glm::quat(1,0,0,0), size);

    gizmo_queue->gizmos.push_back(
        (Gizmo) {
            .mesh_idx = shape,
            .transform = t,
            .color = col,
            .frames = 1
        }
    );
}

void push_gizmo_n_frames(Shapes shape, Transform t, int n, glm::vec4 col) {
    gizmo_queue->gizmos.push_back( 
        (Gizmo) {
            .mesh_idx = shape,
            .transform = t,
            .color = col,
            .frames = n,
        }
    );
}

void pop_gizmo() {
    if (!gizmo_queue->gizmos.empty())
        gizmo_queue->gizmos.pop_back();
}

void end_frame_gizmos() {
    for (int i = (int)(gizmo_queue->gizmos.size()) - 1; i >= 0; --i) {
        if (gizmo_queue->gizmos[i].frames <= 0) {
            gizmo_queue->gizmos.erase(gizmo_queue->gizmos.begin() + i);
        }
    }
}

void render_gizmos(Camera& cam) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    for (Gizmo& gizmo : gizmo_queue->gizmos) {
        // todo: instancing
        gizmo_queue->gizmo_meshes[gizmo.mesh_idx].draw(
            gizmo_queue->gizmo_program,
            gizmo.transform.getModelMat(),
            cam.view,
            cam.proj,
            gizmo.color,
            0
        );

        gizmo.frames--;
    }

    end_frame_gizmos();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}