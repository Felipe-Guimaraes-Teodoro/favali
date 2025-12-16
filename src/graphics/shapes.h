#pragma once

#include "transform.h"
#include "mesh.h"
#include "camera.h"

constexpr float PI = 3.14159265358979323846f;

enum Shapes {
    Square,
    Circle,
    Triangle,
    Cube,
    Sphere,
    Empty, /* WARNING: Empty HAS TO BE the last shape in this enum otherwise 
        gizmos might break
    */
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

struct StaticShape : Shape {
    AABB box;
    bool should_occlude;

    vector<StaticShape*> children;

    StaticShape(Shape&& shape, AABB b, bool occlude)
        : Shape(std::move(shape)), box(b), should_occlude(occlude) {}
};

StaticShape shape_to_static(Shape&& shape);