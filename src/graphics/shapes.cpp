#include "shapes.h"

#include "texture.h"

void Shape::draw(unsigned int program, const Camera& camera) const {
    mesh.draw(program, transform.getModelMat(), camera.view, camera.proj, color, texture);
}

Shape make_shape(Shapes shape, unsigned int texture) {
    vector<float> vertices;
    vector<unsigned int> indices;

    Mesh resulting_mesh = empty_mesh();

    switch (shape) {
        case Square: {
            vertices = {
                // x y z      nx ny nz    u v
                0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
                0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
               -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
               -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f
            };

            indices = {0,1,3, 1,2,3};

            break;
        }

        case Circle: {
            int segments = 32;

            // center vertex (position + normal + uv)
            vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
            vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(1.0f);
            vertices.push_back(0.5f); vertices.push_back(0.5f);

            for (int i = 0; i < segments; i++) {
                float angle = 2.0f * PI * i / segments;
                float x = cosf(angle) * 0.5f;
                float y = sinf(angle) * 0.5f;

                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                // UV coordinates
                vertices.push_back(0.5f + x);
                vertices.push_back(0.5f + y);
            }

            for (int i = 0; i < segments; i++) {
                indices.push_back(0);
                indices.push_back(i + 1);
                indices.push_back(((i + 1) % segments) + 1);
            }

            break;
        }

        case Triangle: {
            vertices = {
                // x y z      nx ny nz    u v
                0.0f,  0.5f, 0.0f,    0.0f, 0.0f, 1.0f,   0.5f, 1.0f,
               -0.5f, -0.5f, 0.0f,    0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
                0.5f, -0.5f, 0.0f,    0.0f, 0.0f, 1.0f,   1.0f, 0.0f
            };

            indices = {0,1,2};
            break;
        }

        case Cube: {
            vertices = {
                // FRONT (+Z)
                -0.5f,-0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
                0.5f,-0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
                0.5f, 0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
                -0.5f, 0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,

                // BACK (−Z)
                0.5f,-0.5f,-0.5f,   0.0f, 0.0f,-1.0f,   0.0f, 0.0f,
                -0.5f,-0.5f,-0.5f,   0.0f, 0.0f,-1.0f,   1.0f, 0.0f,
                -0.5f, 0.5f,-0.5f,   0.0f, 0.0f,-1.0f,   1.0f, 1.0f,
                0.5f, 0.5f,-0.5f,   0.0f, 0.0f,-1.0f,   0.0f, 1.0f,

                // BOTTOM (−Y)
                -0.5f,-0.5f,-0.5f,   0.0f,-1.0f, 0.0f,   0.0f, 0.0f,
                -0.5f,-0.5f, 0.5f,   0.0f,-1.0f, 0.0f,   1.0f, 0.0f,
                0.5f,-0.5f, 0.5f,   0.0f,-1.0f, 0.0f,   1.0f, 1.0f,
                0.5f,-0.5f,-0.5f,   0.0f,-1.0f, 0.0f,   0.0f, 1.0f,

                // TOP (+Y)
                -0.5f, 0.5f, 0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
                0.5f, 0.5f, 0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
                0.5f, 0.5f,-0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
                -0.5f, 0.5f,-0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,

                // LEFT (−X)
                -0.5f,-0.5f,-0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
                -0.5f,-0.5f, 0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
                -0.5f, 0.5f, 0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
                -0.5f, 0.5f,-0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,

                // RIGHT (+X)
                0.5f,-0.5f, 0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
                0.5f,-0.5f,-0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
                0.5f, 0.5f,-0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
                0.5f, 0.5f, 0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f
            };

            indices = {
                // FRONT
                0, 1, 2,   0, 2, 3,

                // BACK
                4, 5, 6,   4, 6, 7,

                // BOTTOM
                10, 9, 8,  11, 10, 8,

                // TOP
                12, 13, 14, 12, 14, 15,

                // LEFT
                16, 17, 18, 16, 18, 19,

                // RIGHT
                20, 21, 22, 20, 22, 23
            };

            break;
        }

        case Sphere: {
            int stacks = 16;
            int slices = 32;
            float radius = 0.5f;

            for (int i = 0; i <= stacks; i++) {
                float stackAngle = PI / 2.0f - i * (PI / (float)stacks);
                float xy = radius * cosf(stackAngle);
                float z = radius * sinf(stackAngle);

                for (int j = 0; j <= slices; j++) {
                    float sliceAngle = j * (2.0f * PI / (float)slices);
                    float x = xy * cosf(sliceAngle);
                    float y = xy * sinf(sliceAngle);

                    // position
                    vertices.push_back(x);
                    vertices.push_back(y);
                    vertices.push_back(z);

                    // normal
                    glm::vec3 n = glm::normalize(glm::vec3(x, y, z));
                    vertices.push_back(n.x);
                    vertices.push_back(n.y);
                    vertices.push_back(n.z);

                    // uv coordinates
                    float u = (float)j / (float)slices;
                    float v = (float)i / (float)stacks;
                    vertices.push_back(u);
                    vertices.push_back(v);
                }
            }

            for (int i = 0; i < stacks; i++) {
                int k1 = i * (slices + 1);
                int k2 = k1 + slices + 1;

                for (int j = 0; j < slices; j++) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);

                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);

                    k1++;
                    k2++;
                }
            }

            break;
        }

        case Empty: {
            break;
        }
    }

    resulting_mesh.vertices = std::move(vertices);
    resulting_mesh.indices = std::move(indices);

    if (!resulting_mesh.vertices.empty() && !resulting_mesh.indices.empty()) {
        setup_mesh(resulting_mesh);

        if (texture == 0)
            texture = create_default_texture();
    }

    return Shape(
        std::move(resulting_mesh),
        Transform::empty(),
        glm::vec4(1.0f),
        texture
    );
}