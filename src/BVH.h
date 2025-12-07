#pragma once

#include "geometry.h"
#include <vector>

struct BVHNode {
    AABB box;
    std::vector<MeshTriangle> tris;
    BVHNode* left = nullptr;
    BVHNode* right = nullptr;
    bool isLeaf = false;
};

void bvhQuery(BVHNode* node, const AABB& box, std::vector<MeshTriangle>& out);

BVHNode* buildBVH(const std::vector<MeshTriangle>& tris, int depth = 0);
