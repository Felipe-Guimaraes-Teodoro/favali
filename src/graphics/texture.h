#pragma once

#include "stb_image.h"
#include "glad/glad.h"
#include <vector>

unsigned int create_default_texture();

unsigned int make_texture(const char *path);

unsigned int make_texture_from_memory(const uint8_t* src, int size);

unsigned int make_cube_map_texture(const std::vector<const char*> &faces);
