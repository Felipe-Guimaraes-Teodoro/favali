#pragma once

#include "transform.h"
#include "mesh.h"
#include "camera.h"
#include "glm.hpp"
#include "texture.h"
#include <vector>
#include <cmath>
using std::vector;

constexpr float PI = 3.14159265358979323846f;

enum Shapes {
    Square,
    Circle,
    Triangle,
    Cube,
    Sphere,
    Empty,
};

typedef struct {
    Mesh mesh;
    Transform transform;
    glm::vec4 color;
    unsigned int texture;

    void draw(unsigned int program, const Camera& camera) const;
} Shape;

Shape make_shape(Mesh& mesh, Transform& transform, unsigned int texture);
Shape make_shape(Shapes shape, unsigned int texture = 0);