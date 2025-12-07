#include "BVH.h"
#include "glm.hpp"
#include <algorithm>
#include <limits>
#include <iostream> // só pra debug, remove depois

static bool is_finite_vec3(const glm::vec3 &v){
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

BVHNode* buildBVH(const std::vector<MeshTriangle>& tris, int depth){
    BVHNode* node = new BVHNode();

    // se não tem tri nada, vira folha vazio
    if (tris.empty()){
        node->isLeaf = true;
        node->tris = tris;
        node->left = node->right = nullptr;
        node->box.min = glm::vec3(0.0f);
        node->box.max = glm::vec3(0.0f);
        return node;
    }

    // calc AABB cobrindo todos os tris
    glm::vec3 minv(  std::numeric_limits<float>::infinity());
    glm::vec3 maxv(-std::numeric_limits<float>::infinity());

    for(const auto& t : tris){
        AABB b = makeAABB(t);
        // sanity check - se tiver NaN/inf aborta cedo (debug)
        if (!is_finite_vec3(b.min) || !is_finite_vec3(b.max)){
            std::cerr << "buildBVH: found non-finite triangle AABB (depth " << depth << ")\n";
            node->isLeaf = true;
            node->tris = tris;
            return node;
        }
        minv = glm::min(minv, b.min);
        maxv = glm::max(maxv, b.max);
    }

    node->box.min = minv;
    node->box.max = maxv;

    // criterio de parada básico
    if (tris.size() <= 16 || depth >= 20){
        node->isLeaf = true;
        node->tris = tris;
        node->left = node->right = nullptr;
        return node;
    }

    // escolhe eixo maior
    glm::vec3 size = maxv - minv;
    int axis = 0;
    if (size.y > size.x && size.y > size.z) axis = 1;
    else if (size.z > size.x) axis = 2;

    // cópia mutável pra ordenar
    std::vector<MeshTriangle> local = tris;

    std::sort(local.begin(), local.end(), [&](const MeshTriangle& t1, const MeshTriangle& t2){
        float c1 = (t1.a[axis] + t1.b[axis] + t1.c[axis]) / 3.0f;
        float c2 = (t2.a[axis] + t2.b[axis] + t2.c[axis]) / 3.0f;
        return c1 < c2;
    });

    int mid = int(local.size() / 2);

    // se o split não dividir (por exemplo mid==0 ou mid==n), vira leaf pra evitar recursão infinita
    if (mid == 0 || mid == (int)local.size()){
        node->isLeaf = true;
        node->tris = local;
        node->left = node->right = nullptr;
        return node;
    }

    std::vector<MeshTriangle> leftTris (local.begin(), local.begin() + mid);
    std::vector<MeshTriangle> rightTris(local.begin() + mid, local.end());

    // checagem extra: se um dos lados ficou vazio, torna folha (robustez)
    if (leftTris.empty() || rightTris.empty()){
        node->isLeaf = true;
        node->tris = local;
        node->left = node->right = nullptr;
        return node;
    }

    node->left  = buildBVH(leftTris,  depth+1);
    node->right = buildBVH(rightTris, depth+1);

    return node;
}

void bvhQuery(BVHNode* node, const AABB& box, std::vector<MeshTriangle>& out){
    if (!node) return;
    if (!intersects(node->box, box)) return;

    if (node->isLeaf){
        for(auto& t : node->tris)
            out.push_back(t);
        return; // make all leaf tris candidates
    }

    bvhQuery(node->left,  box, out);
    bvhQuery(node->right, box, out);
}
