#include "npc.h"

#include <stdio.h>

#include "gizmos.h"
#include "player.h"
#include "raycast.h"

struct ivec3_hash {
    std::size_t operator()(const glm::ivec3& v) const {
        std::size_t hx = std::hash<int>()(v.x);
        std::size_t hy = std::hash<int>()(v.y);
        std::size_t hz = std::hash<int>()(v.z);

        return hx ^ (hy << 1) ^ (hz << 2);
    }
};


NavGraph *global_navGraph = nullptr;
constexpr vec3 grid_origin = vec3(0.0f, 0.5f, 0.0f);

NavNode* create_navnode(vec3 position) {
    NavNode* n = new NavNode();
    n->pos = position;

    return n;
}

NavGraph* create_navgraph(vector<BVHNode*>& bvh) {
    NavGraph* graph = new NavGraph();
    
    return graph;
}

constexpr float cellSize = 1;

glm::ivec3 get_grid_key(const glm::vec3& nodePos) {
    return glm::ivec3(
        int(std::floor(nodePos.x / cellSize)),
        int(std::floor(nodePos.y / cellSize)),
        int(std::floor(nodePos.z / cellSize))
    );
}

void expand_navgraph(NavGraph *ng, vector<BVHNode*>& bvh, vec3 pos, vec3 goal) {
    // determine bounds
    vec3 minBound = glm::min(pos, goal);
    vec3 maxBound = glm::max(pos, goal);

    int nx = static_cast<int>(glm::ceil((maxBound.x - minBound.x) / cellSize));
    int ny = static_cast<int>(glm::ceil((maxBound.y - minBound.y) / cellSize));
    int nz = static_cast<int>(glm::ceil((maxBound.z - minBound.z) / cellSize));

    vector<AABB> boxes;
    for (BVHNode* node : bvh) {
        bvhQueryAABB(node, boxes);
    }

    // map from grid key to node index
    std::unordered_map<glm::ivec3, int, ivec3_hash> gridMap;
    std::vector<int> added_node_indices;

    // neighbor offsetts
    ivec3 dirs[18] = {
        ivec3(1, 0, 0), ivec3(-1, 0, 0),
        ivec3(0, 1, 0), ivec3(0, -1, 0),
        ivec3(0, 0, 1), ivec3(0, 0, -1),

        ivec3(1, 1, 0), ivec3(-1, 1, 0),
        ivec3(1, -1, 0), ivec3(-1, -1, 0),
        ivec3(1, 0, 1), ivec3(-1, 0, 1),
        ivec3(1, 0, -1), ivec3(-1, 0, -1),
        ivec3(0, 1, 1), ivec3(0, -1, 1),
        ivec3(0, 1, -1), ivec3(0, -1, -1)
    };

    // create nodes
    for (int i = 0; i < nx * ny * nz; i++) {
        int x = i % nx;
        int y = (i / nx) % ny;
        int z = i / (nx * ny);

        vec3 nodePos = glm::vec3(
            std::floor((minBound.x + x * cellSize) / cellSize) * cellSize,
            std::floor((minBound.y + y * cellSize) / cellSize) * cellSize,
            std::floor((minBound.z + z * cellSize) / cellSize) * cellSize
        );

        glm::ivec3 key = get_grid_key(nodePos);

        // skip duplicates
        if (gridMap.count(key) > 0) continue;

        bool blocked = false;
        for (const AABB& box : boxes) {
            if (is_point_on_aabb(box, nodePos)) {
                blocked = true;
                break;
            }
        }

        if (!blocked) {
            int idx = ng->nodes.size();
            ng->nodes.push_back(create_navnode(nodePos));
            gridMap[key] = idx;
            added_node_indices.push_back(idx);
        }
    }

    // link neighbors
    for (int idx : added_node_indices) {
        NavNode* node = ng->nodes[idx];

        glm::ivec3 key = get_grid_key(node->pos);

        for (const ivec3& dir : dirs) {
            glm::ivec3 neighborKey = key + dir;
            auto it = gridMap.find(neighborKey);
            if (it != gridMap.end()) {
                NavNode* neighbor = ng->nodes[it->second];

                // avoid duplicate neighbor entries
                if (!(std::find(node->neighbors.begin(), node->neighbors.end(), neighbor) == node->neighbors.end()))
                    continue;
                
                for (const AABB& box : boxes) {
                    if (is_line_on_aabb(box, node->pos, neighbor->pos)) {
                        break;
                    }
                }

                node->neighbors.push_back(neighbor);

                /*
                if (std::find(neighbor->neighbors.begin(), neighbor->neighbors.end(), node) == neighbor->neighbors.end()) {
                    neighbor->neighbors.push_back(node);
                }
                */
            }
        }
    }

}

vector<NavNode*> *find_path(NavGraph *ng) {
    
}

void visualize_nodes(vector<NavNode*> nodes) {
    for (NavNode* n : nodes) {
        push_gizmo(Shapes::Cube, Transform(n->pos, vec3(0.05f)));

        for (NavNode* neighbor : n->neighbors) {
            glm::vec3 start = glm::vec3(n->pos);
            glm::vec3 end   = glm::vec3(neighbor->pos);

            glm::vec3 dir = end - start;
            float length = glm::length(dir);
            dir = glm::normalize(dir);

            glm::quat rot = rotate_from_to(glm::vec3(0.0f, 0.0f, 1.0f), dir);

            glm::vec3 mid = start + dir * (length * 0.5f);

            glm::vec3 scale(0.1f, 0.1f, length);

            push_gizmo(Shapes::Cube, Transform(mid, rot, scale));
        }
    }
}

void visualize_path(vector<NavNode*> nodes) {
    int i = 0;
    for (NavNode* n : nodes) {
        push_gizmo(Shapes::Cube, Transform(n->pos, vec3(0.05f)));

        if (i < nodes.size() - 1) {
            NavNode* neighbor = nodes[i+1];
            glm::vec3 start = glm::vec3(n->pos);
            glm::vec3 end   = glm::vec3(neighbor->pos);

            glm::vec3 dir = end - start;
            float length = glm::length(dir);
            dir = glm::normalize(dir);

            glm::quat rot = rotate_from_to(glm::vec3(0.0f, 0.0f, 1.0f), dir);

            glm::vec3 mid = start + dir * (length * 0.5f);

            glm::vec3 scale(0.1f, 0.1f, length);

            push_gizmo(Shapes::Cube, Transform(mid, rot, scale));
        }

        i++;
    }
}

Npc create_npc() {
    Npc n = {};

    n.jump_force = 5.0;
    n.last_jumped = 0.0;
    n.grounded = false;
    n.position = vec3(0.0, 3.0, 0.0);

    n.collider.radius = 1.0;
    n.collider.center = n.position;
    n.speed = 5.0;
    n.cant_find_route = true;

    return n;
}

void npc_solve_collisions(Npc& npc, vector<BVHNode*> worldBVHs) {
    std::vector<MeshTriangle> candidates;
    AABB query;
    query.min = npc.position - glm::vec3(npc.collider.radius);
    query.max = npc.position + glm::vec3(npc.collider.radius); // create bounding box roughly the size of the npc

    for (int i = 0; i < worldBVHs.size(); i++){
        bvhQuery(worldBVHs[i], query, candidates); // get possible triangles to intersect

        for (auto& tri : candidates) {
            // closest tri to point
            glm::vec3 closest = closestPointOnTriangle(npc.position, tri);

            glm::vec3 triNormal = tri.getTriangleNormal();

            // push_gizmo(Shapes::Sphere, Transform(closest, vec3(0.1)), 1, {0.0, 0.0, 1.0, 1.0});

            glm::vec3 diff = npc.position - closest;
            float dist = length(diff);

            if (dist < npc.collider.radius) {
                float push = npc.collider.radius - dist;

                // push npc by the tri's normal
                npc.position += triNormal * push;
            }
        }
        candidates.clear();
    }

    push_gizmo(Shapes::Cube, query, 1, {1.0, 1.0, 0.0, 1.0});

    Ray ray = { npc.position, -UP, npc.collider.radius * 2.0f };
    AABB feet = makeAABB_from_ray(ray);

    npc.grounded = false;

    for (int i = 0; i < worldBVHs.size(); i++){
        bvhQuery(worldBVHs[i], feet, candidates);

        for (auto& tri : candidates) {
            glm::vec3 feetCenter = (feet.min + feet.max) * 0.5f;
            glm::vec3 closest = closestPointOnTriangle(feetCenter, tri);
            
            glm::vec3 diff = feetCenter - closest;
            float dist = length(diff);

            if (dist < npc.collider.radius * 0.5) {
                npc.grounded = true;
            }
        }
        candidates.clear();
    }

    if (!global_navGraph) {
        // global_navGraph = create_navgraph();

        return;
    }

    if (npc.cant_find_route) {
        npc.cant_find_route = false;
        expand_navgraph(global_navGraph, worldBVHs, npc.position, npc.goal);
    }
}

void update_npc(Npc& npc, float dt) {
    npc.velocity.y += GRAVITY.y * dt;

    if (npc.grounded) {
        npc.velocity.y = 0.0f;
    }

    glm::vec3 dir = npc.goal - npc.position;
    dir.y = 0.0f;

    if (glm::length2(dir) > 0.5) {
        npc.wishdir = glm::normalize(dir);
    } else {
        npc.wishdir = glm::vec3(0.0f);
    }

    npc.velocity.x = npc.wishdir.x * npc.speed;
    npc.velocity.z = npc.wishdir.z * npc.speed;

    npc.position += npc.velocity * dt;

    printf("pos %f %f %f\n", npc.position.x, npc.position.y, npc.position.z);

    if (!global_navGraph) {
        // global_navGraph = create_navgraph();

        return;
    }
}
