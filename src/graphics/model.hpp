#pragma once

#include "shapes.h"
#include "level.h"

Shape create_shape_from_gltf(const char *path, int idx);
Level *create_level_from_gltf(const char *path);
Model *create_model_from_gltf(const char *path);