#pragma once

#include "stb_image.h"
#include "glad/glad.h"
#include <vector>

unsigned int create_default_texture();

unsigned int create_texture(const char *path);

unsigned int create_texture_from_memory(const uint8_t* src, int size);

unsigned int create_cube_map_texture(const std::vector<const char*> &faces);

unsigned int create_random_texture();

unsigned int create_perlin_texture2D(int size = 512);

unsigned int create_perlin_texture3D(int size = 512);
