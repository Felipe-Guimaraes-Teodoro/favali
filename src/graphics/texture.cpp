#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"
#include <string>

// todo: whenever this is called return A default texture
// already stored somewhere instead of always creating the 
// same white thing
unsigned int create_default_texture() {
    unsigned char white_pixel[] = {255, 255, 255, 255}; // RGBA white
    
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture_id;
}

unsigned int make_texture(const char *path){
    // stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

    GLenum format;
    if (nrChannels == 1) format = GL_RED;
    else if (nrChannels == 3) format = GL_RGB;
    else if (nrChannels == 4) format = GL_RGBA;

    unsigned int texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        printf("Failed to load texture %s. Error: %s\n", path, stbi_failure_reason());
    }

    stbi_image_free(data);

    return texture;
}

unsigned int make_texture_from_memory(const uint8_t* src, int size) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load_from_memory(src, size, &width, &height, &nrChannels, 0);

    unsigned int texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 4) format = GL_RGBA;
        else if (nrChannels == 1) format = GL_RED;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        printf("Failed to load texture %p. Error: %s\n", src, stbi_failure_reason());
    }
    
    stbi_image_free(data);

    return texture;
}

unsigned int make_cube_map_texture(const std::vector<const char*> &faces){
    unsigned int texture;
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    
    int width, height, nrChannels;
    for (int i = 0; i < faces.size(); i++){
        unsigned char *data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
        
        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;
        
        if (data){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else{
            printf("Failed to load cubemap texture %s. Error: %s\n", faces[i], stbi_failure_reason());
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture;
}
