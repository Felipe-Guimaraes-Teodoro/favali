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

struct Shape {
    Mesh mesh;
    Transform transform;
    glm::vec4 color = glm::vec4(1.0f);
    unsigned int texture = 0;

    Shape(Mesh&& m, Transform t, glm::vec4 c, unsigned int tex)
        : mesh(std::move(m)), transform(t), color(c), texture(tex) {}
        
    // prevent copying
    Shape(const Shape&) = delete;
    Shape& operator=(const Shape&) = delete;

     // allow moving
    Shape(Shape&& other) noexcept
        : mesh(std::move(other.mesh)),
          transform(std::move(other.transform)),
          color(other.color),
          texture(other.texture)
    {
        other.texture = 0;
    }

    Shape& operator=(Shape&& other) noexcept {
        if (this != &other) {
            mesh = std::move(other.mesh);
            transform = std::move(other.transform);
            color = other.color;
            texture = other.texture;
            other.texture = 0;
        }
        return *this;
    }

    void draw(unsigned int program, const Camera& camera) const;
};

Shape make_shape(Shapes shape, unsigned int texture = 0);