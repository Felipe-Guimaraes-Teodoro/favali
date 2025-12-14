#include "mesh.h"
#include "texture.h"
#include "shaders.h"

Mesh::~Mesh() {
    destroy();
}

void Mesh::destroy() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);

    VBO = VAO = EBO = 0; // bro i had no idea this could be done
    setup = false;
}

void Mesh::draw(
    unsigned int program, 
    glm::mat4 model_mat, 
    glm::mat4 view_mat, 
    glm::mat4 proj_mat, 
    glm::vec4 col, 
    unsigned int texture,
    unsigned int draw_mode // = GL_TRIANGLES by default
) const {
    if (!setup) {
        printf("WARNING: Attempting to draw deleted or incomplete mesh\n"); 
        return;
    }

    glUseProgram(program);
    
    shader_uniform_mat4(program, "model", model_mat);
    shader_uniform_mat4(program, "view", view_mat);
    shader_uniform_mat4(program, "projection", proj_mat);

    shader_uniform_vec4(program, "color", col);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawElements(
        draw_mode,
        indices.size(), 
        GL_UNSIGNED_INT, 
        0
    );
}

void setup_mesh(Mesh& mesh) {
    mesh.setup = true;

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);
    
    glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
            glBufferData(GL_ARRAY_BUFFER, 
                mesh.vertices.size() * sizeof(float), 
                mesh.vertices.data(), 
                GL_STATIC_DRAW
            );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                mesh.indices.size() * sizeof(unsigned int), 
                mesh.indices.data(), 
                GL_STATIC_DRAW
            );

        // POSITION
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // NORMAL
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // UV
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
}

Mesh create_mesh(vector<float>& vertices, vector<unsigned int>& indices) {
    Mesh mesh = {};
    mesh.vertices = vertices; // POSTIION, normal, uv
    mesh.indices = indices;
    
    setup_mesh(mesh);

    return mesh;
}

Mesh empty_mesh() {
    Mesh mesh = {};

    return mesh;
}


// instanced mesh

InstancedMesh::~InstancedMesh() {
    destroy();
}

void InstancedMesh::destroy() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (IBO) glDeleteBuffers(1, &IBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);

    VBO = VAO = EBO = IBO = 0;
    setup = false;
}

void InstancedMesh::draw(
    unsigned int program,
    glm::mat4 view_mat, 
    glm::mat4 proj_mat, 
    glm::vec4 col,
    unsigned int texture,
    unsigned int draw_mode // = GL_TRIANGLES by default
) const {
    if (!setup) {
        printf("WARNING: Attempting to draw deleted or incomplete mesh\n"); 
        return;
    }

    glUseProgram(program);
    
    shader_uniform_mat4(program, "view", view_mat);
    shader_uniform_mat4(program, "projection", proj_mat);

    shader_uniform_vec4(program, "color", col);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawElementsInstanced(
        draw_mode,
        indices.size(), 
        GL_UNSIGNED_INT, 
        0,
        instances.size()
    );
}

void setup_mesh_instanced(InstancedMesh& mesh) {
    mesh.setup = true;

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);
    glGenBuffers(1, &mesh.IBO);
    
    glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
            glBufferData(GL_ARRAY_BUFFER, 
                mesh.vertices.size() * sizeof(float), 
                mesh.vertices.data(), 
                GL_STATIC_DRAW
            );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                mesh.indices.size() * sizeof(unsigned int), 
                mesh.indices.data(), 
                GL_STATIC_DRAW
            );

            // POSITION
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // NORMAL
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // UV
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

        // instances
        glBindBuffer(GL_ARRAY_BUFFER, mesh.IBO);
            glBufferData(
                GL_ARRAY_BUFFER, 
                mesh.instances.size() * sizeof(float) * 16, 
                mesh.instances.data(),
                GL_STATIC_DRAW // GL_DYNAMIC_DRAW
            );

            // 4 * vector4
            for (int i = 0; i < 4; i++) {
                glVertexAttribPointer(
                    3 + i,
                    4,
                    GL_FLOAT,
                    GL_FALSE,
                    sizeof(float) * 16,
                    (void*)(i * sizeof(float) * 4)
                );
                glEnableVertexAttribArray(3 + i);
                glVertexAttribDivisor(3 + i, 1);
            }
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
}

InstancedMesh create_mesh_instanced(
    std::vector<float>& vertices,
    std::vector<unsigned int>& indices,
    std::vector<glm::mat4>& instances
) {
    InstancedMesh mesh;

    mesh.vertices = vertices;
    mesh.indices  = indices;
    mesh.instances = instances;
    mesh.last_instance_buffer_size = instances.size();

    setup_mesh_instanced(mesh);
    return mesh;
}

InstancedMesh mesh_to_instanced(Mesh &mesh) {
    InstancedMesh instanced_mesh;

    instanced_mesh.vertices = mesh.vertices;
    instanced_mesh.indices = mesh.indices;
    mesh.destroy();

    vector<glm::mat4> instances = {glm::identity<glm::mat4>()};

    instanced_mesh.instances = instances;
    instanced_mesh.last_instance_buffer_size = 1;

    setup_mesh_instanced(instanced_mesh);
    return instanced_mesh;
}

void update_instances(InstancedMesh& mesh) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh.IBO);

    if (mesh.last_instance_buffer_size != mesh.instances.size()) {
        glBufferData(
            GL_ARRAY_BUFFER,
            mesh.instances.size() * sizeof(float) * 16,
            mesh.instances.data(),
            GL_STATIC_DRAW // GL_DYNAMIC_DRAW
        );
    } else {
        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            mesh.instances.size() * sizeof(float) * 16,
            mesh.instances.data()
        );
    }

    mesh.last_instance_buffer_size = mesh.instances.size();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void push_instance(InstancedMesh& mesh, const glm::mat4& instance) {
    mesh.instances.push_back(instance);
}

void pop_instance(InstancedMesh& mesh) {
    mesh.instances.pop_back();
}

/// doesnt actually clear, just sets the instances to a single identity matrix
void clear_instances(InstancedMesh& mesh) {
    mesh.instances = {glm::identity<glm::mat4>()};
}

// Cube Map business

CubeMapMesh create_cube_map() {
    // make actual cube
    std::vector<float> vertices = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,
        3, 4, 5,

        6, 7, 8,
        9, 10, 11,

        12, 13, 14,
        15, 16, 17,

        18, 19, 20,
        21, 22, 23,

        24, 25, 26,
        27, 28, 29,

        30, 31, 32,
        33, 34, 35
    };
    
    CubeMapMesh mesh;
    
    mesh.vertices = vertices;
    mesh.indices  = indices;
    setup_cube_map_mesh(mesh);

    // make sun
    std::vector<float> sun_vertices = {
        // pos              // uv
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
        0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
    };

    std::vector<unsigned int> sun_indices = {
        0, 1, 2,
        2, 3, 0
    };

    mesh.sun.vertices = sun_vertices;
    mesh.sun.indices = sun_indices;
    
    mesh.sun.setup = true;

    glGenVertexArrays(1, &mesh.sun.VAO);
    glGenBuffers(1, &mesh.sun.VBO);
    glGenBuffers(1, &mesh.sun.EBO);
    
    glBindVertexArray(mesh.sun.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.sun.VBO);
    glBufferData(GL_ARRAY_BUFFER, 
        mesh.sun.vertices.size() * sizeof(float), 
        mesh.sun.vertices.data(), 
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.sun.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
        mesh.sun.indices.size() * sizeof(unsigned int), 
        mesh.sun.indices.data(), 
        GL_STATIC_DRAW
    );

    // POSITION
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // UVs
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);

    return mesh;
}

void setup_cube_map_mesh(CubeMapMesh& mesh) {
    mesh.setup = true;

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);
    
    glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
            glBufferData(GL_ARRAY_BUFFER, 
                mesh.vertices.size() * sizeof(float), 
                mesh.vertices.data(), 
                GL_STATIC_DRAW
            );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                mesh.indices.size() * sizeof(unsigned int), 
                mesh.indices.data(), 
                GL_STATIC_DRAW
            );

        // POSITION
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
}

CubeMapMesh::~CubeMapMesh() {
    destroy();
}

void CubeMapMesh::destroy() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);

    VBO = VAO = EBO = 0;
    setup = false;
}

void CubeMapMesh::draw(
    unsigned int sky_program,
    unsigned int sun_program,
    Camera& camera,
    float time
) const {
    float t = time * 0.05f;

    glm::vec3 sunDir = glm::normalize(glm::vec3(
        cos(t),
        sin(t),
        0.2f
    ));

    float sunIntensity = glm::clamp(glm::abs(sunDir.y) * 0.5f + 0.5f, 0.0f, 1.0f);
    glm::vec3 sunColor = vec3(1.0, 0.95, 0.8);

    // draw skybox
    if (!setup) {
        printf("WARNING: Attempting to draw deleted or incomplete cube map mesh\n"); 
        return;
    }

    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    glUseProgram(sky_program);

    shader_uniform_mat4(sky_program, "model", glm::mat4(1.0f));
    shader_uniform_mat4(sky_program, "view", glm::mat4(glm::mat3(camera.view)));
    shader_uniform_mat4(sky_program, "projection", camera.proj);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, day_texture);
    glUniform1i(glGetUniformLocation(sky_program, "daySky"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, night_texture);
    glUniform1i(glGetUniformLocation(sky_program, "nightSky"), 1);

    glActiveTexture(GL_TEXTURE0);

    shader_uniform_vec3(sky_program, "sunDir", sunDir);
    shader_uniform_vec3(sky_program, "sunColor", sunColor);
    shader_uniform_float(sky_program, "sunIntensity", sunIntensity);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // draw sun quad
    if (!sun.setup) {
        printf("WARNING: Attempting to draw deleted or incomplete sun mesh from cube map\n"); 
        return;
    }

    auto drawBillboard = [&](glm::vec3 dir, glm::vec3 color, float intensity, float scale) {
        glm::vec3 pos = camera.position + normalize(dir) * 500.0f;
        glm::vec3 toCam = normalize(camera.position - pos);
        glm::vec3 right = normalize(cross(glm::vec3(0.0f, 1.0f, 0.0f), toCam));
        glm::vec3 up    = cross(toCam, right);

        glm::mat4 model(1.0f);
        model[0] = glm::vec4(right, 0.0f);
        model[1] = glm::vec4(up, 0.0f);
        model[2] = glm::vec4(toCam, 0.0f);
        model[3] = glm::vec4(pos, 1.0f);
        model = glm::scale(model, glm::vec3(scale));

        glUseProgram(sun_program);
        shader_uniform_mat4(sun_program, "model", model);
        shader_uniform_mat4(sun_program, "view", camera.view);
        shader_uniform_mat4(sun_program, "projection", camera.proj);
        shader_uniform_vec3(sun_program, "sunColor", color);
        shader_uniform_float(sun_program, "sunIntensity", intensity);

        glBindVertexArray(sun.VAO);
        glDrawElements(GL_TRIANGLES, sun.indices.size(), GL_UNSIGNED_INT, 0);
    };

    // enable blending for sun/moon
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    // draw sun
    drawBillboard(sunDir, glm::vec3(1.0, 0.95, 0.8), sunIntensity, 50.0f);

    // draw moon
    glm::vec3 moonDir = -sunDir;
    float moonIntensity = sunIntensity;
    drawBillboard(moonDir, glm::vec3(0.6, 0.65, 0.8), moonIntensity, 35.0f);

    // restore depth/blend
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}
