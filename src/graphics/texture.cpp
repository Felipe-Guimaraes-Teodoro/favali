#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"
#include "PerlinNoise.hpp"
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

unsigned int make_random_texture(){
    int size = 256;
    std::vector<unsigned char> data(size*size);

    for(int y=0; y<size; y++){
        for(int x=0; x<size; x++){
            data[y*size + x] = rand() % 256;
        }
    }

    GLuint noiseTex;
    glGenTextures(1, &noiseTex);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size, size, 0, GL_RED, GL_UNSIGNED_BYTE, data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    return noiseTex;
}

using namespace siv;

unsigned int make_perlin_texture2D(int size){
    siv::PerlinNoise perlin(1234);
    std::vector<unsigned char> data(size*size);

    int octaves = 5;

    for(int y=0; y<size; y++){
        for(int x=0; x<size; x++){
            double nx = double(x)/double(size);
            double ny = double(y)/double(size);

            double n = 0.0;
            double freq = 5.0;
            double amp = 1.0;
            double maxAmp = 0.0;
            double damp = 0.5;

            // fractal noise 5 octaves
            for(int o=0; o<octaves; o++){
                n += amp * perlin.noise2D_01(nx*freq, ny*freq);
                maxAmp += amp;
                freq *= 2.0;
                amp *= damp;
            }

            // normaliza pra 0..1
            n /= maxAmp;

            // opcional: tweak contraste pra deixar nuvens mais fofas
            n = pow(n, 2.5); // aumenta contraste, ajusta se quiser nuvens mais suaves

            data[y*size + x] = static_cast<unsigned char>(n * 255);
        }
    }

    GLuint noiseTex;
    glGenTextures(1, &noiseTex);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size, size, 0, GL_RED, GL_UNSIGNED_BYTE, data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return noiseTex;
}

unsigned int make_perlin_texture3D(int size){
    siv::PerlinNoise perlin(1234);
    std::vector<unsigned char> data(size*size*size);

    int octaves = 5;

    for(int z=0; z<size; z++){
        for(int y=0; y<size; y++){
            for(int x=0; x<size; x++){
                double nx = double(x)/double(size);
                double ny = double(y)/double(size);
                double nz = double(z)/double(size);

                double n = 0.0;
                double freq = 1.0;
                double amp = 0.1;
                double maxAmp = 0.0;

                for(int o=0; o<octaves; o++){
                    n += amp * perlin.noise3D_01(nx*freq, ny*freq, nz*freq);
                    maxAmp += amp;
                    freq *= 2.0;
                    amp *= 0.5;
                }

                n /= maxAmp;
                n = pow(n, 1.5);

                data[z*size*size + y*size + x] = static_cast<unsigned char>(n * 255);
            }
        }
    }

    GLuint noiseTex;
    glGenTextures(1, &noiseTex);
    glBindTexture(GL_TEXTURE_3D, noiseTex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, size, size, size, 0, GL_RED, GL_UNSIGNED_BYTE, data.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return noiseTex;
}
