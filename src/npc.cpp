#include "npc.h"

#include <stdio.h>
#include <optional>
#include <queue>
#include <algorithm>

#include "gizmos.h"
#include "player.h"
#include "raycast.h"

NavGraph *global_navGraph = nullptr;
constexpr vec3 grid_origin = vec3(0.0f, 0.5f, 0.0f);

NavNode* create_navnode() {
    NavNode* n = new NavNode();

    return n;
}

NavGraph* create_navgraph(vector<BVHNode*>& bvh) {
    NavGraph* graph = new NavGraph();
    
    return graph;
}

constexpr float cellSize = 2;

ivec3 find_closest_node(NavGraph *ng, vec3 position) {
    float closest_dist = std::numeric_limits<float>::max();
    ivec3 closest_node = {0, 0, 0};

    for (const auto &pair : ng->nodes) {
        const ivec3 &node_pos = pair.first;
        vec3 node_center(node_pos.x, node_pos.y, node_pos.z);

        float dist = glm::length(node_center - position);
        if (dist < closest_dist) {
            closest_dist = dist;
            closest_node = node_pos;
        }
    }

    return closest_node;
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

    std::vector<ivec3> added_node_indices;

    // neighbor offsets
    ivec3 dirs[18] = {
        ivec3(cellSize, 0, 0), ivec3(-cellSize, 0, 0),
        ivec3(0, cellSize, 0), ivec3(0, -cellSize, 0),
        ivec3(0, 0, cellSize), ivec3(0, 0, -cellSize),

        ivec3(cellSize, cellSize, 0), ivec3(-cellSize, cellSize, 0),
        ivec3(cellSize, -cellSize, 0), ivec3(-cellSize, -cellSize, 0),
        ivec3(cellSize, 0, cellSize), ivec3(-cellSize, 0, cellSize),
        ivec3(cellSize, 0, -cellSize), ivec3(-cellSize, 0, -cellSize),
        ivec3(0, cellSize, cellSize), ivec3(0, -cellSize, cellSize),
        ivec3(0, cellSize, -cellSize), ivec3(0, -cellSize, -cellSize)
    };

    // create nodes
    for (int i = 0; i < nx * ny * nz; i++) {
        int x = i % nx;
        int y = (i / nx) % ny;
        int z = i / (nx * ny);

        ivec3 nodePos = glm::vec3(
            std::floor((minBound.x + x * cellSize) / cellSize) * cellSize,
            std::floor((minBound.y + y * cellSize) / cellSize) * cellSize,
            std::floor((minBound.z + z * cellSize) / cellSize) * cellSize
        );

        ivec3 closest = find_closest_node(ng, nodePos);

        // skip duplicates
        if (ng->nodes.find(nodePos) != ng->nodes.end()) {
            continue;
        }


        bool blocked = false;
        for (const AABB& box : boxes) {
            float dist_from_ground = 0.0;
            float dist_from_ceiling = FLT_MAX;

            Ray to_ground{vec3(nodePos), -UP};
            Ray to_ceiling{vec3(nodePos), UP};
            
            float min_dist = FLT_MAX;
            if (ray_intersects_aabb(box, to_ground, min_dist)) {
                dist_from_ground = min_dist;
            }

            // todo: instead of checking if a point intersects this box
            // check if the npc's collider fits
            if (is_point_on_aabb(box, nodePos) ||
                dist_from_ground > cellSize * 2 ||
                ray_intersects_aabb(box, to_ceiling, dist_from_ceiling) && dist_from_ceiling < cellSize   
            ) {
                blocked = true;
                break;
            }
        }

        if (!blocked) {
            ng->nodes.emplace(nodePos, create_navnode());
            added_node_indices.push_back(nodePos);
        }
    }

    // link neighbors
    for (ivec3 idx : added_node_indices) {
        NavNode* node = ng->nodes[idx];

        for (const ivec3& dir : dirs) {
            glm::ivec3 neighborKey = idx + dir;
            auto it = ng->nodes.find(neighborKey);
            if (it != ng->nodes.end()) {
                NavNode* neighbor = ng->nodes[neighborKey];

                // avoid duplicate neighbor entries
                if (std::find(node->neighbors.begin(), node->neighbors.end(), neighborKey) != node->neighbors.end())
                    continue;
                
                bool blocked = false;
                for (const AABB& box : boxes) {
                    if (is_line_on_aabb(box, vec3(idx), vec3(neighborKey))) {
                        blocked = true;
                        break;
                    }
                }

                if (!blocked) {
                    // add neighbor
                    node->neighbors.push_back(neighborKey);

                    // and do it two-way if not already
                    if (std::find(neighbor->neighbors.begin(), neighbor->neighbors.end(), idx) == neighbor->neighbors.end()) {
                        neighbor->neighbors.push_back(idx);
                    }
                }

            }
        }
    }
}

//bfs to search optimal path
vector<ivec3> find_path(NavGraph *ng, vec3 start_pos, vec3 end) {
    unordered_map<ivec3, bool, ivec3_hash, ivec3_equal> visited;
    unordered_map<ivec3, int, ivec3_hash, ivec3_equal> distance;
    unordered_map<ivec3, ivec3, ivec3_hash, ivec3_equal> parent;

    std::queue<ivec3> queue;

    ivec3 start = find_closest_node(ng, ivec3(start_pos));
    ivec3 goal = find_closest_node(ng, ivec3(end));

    queue.push(start);
    visited[start] = true;
    distance[start] = 0;

    bool found = false;

    while(!queue.empty() && !found) {
        ivec3 node = queue.front();
        queue.pop();

        // does node even exist?
        if (ng->nodes.find(node) == ng->nodes.end()) {
            continue;
        }

        for (const ivec3 &neighbor : ng->nodes[node]->neighbors) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                parent[neighbor] = node;
                distance[neighbor] = distance[node] + 1;
                queue.push(neighbor);

                if (neighbor == goal) {
                    found = true;
                    break;
                }
            }
        }
    }

    vector<ivec3> path;
    if (!visited[goal]) 
        return path; // no path found

    // Reconstruct path from goal to start
    ivec3 current = goal;
    while (true) {
        path.push_back(current);
        if (current == start) break;
        current = parent[current];
    }

    std::reverse(path.begin(), path.end());
    return path;
}

void visualize_nodes(const NavGraph* graph) {
    for (const auto& [pos_i, node] : graph->nodes) {
        vec3 pos = vec3(pos_i);

        push_gizmo(Shapes::Cube, Transform(pos, vec3(0.05f)));

        for (const ivec3& neighbor_i : node->neighbors) {
            vec3 start = pos;
            vec3 end = vec3(neighbor_i);

            vec3 dir = end - start;
            float length = glm::length(dir);
            if (length <= 0.0001f)
                continue;

            dir = glm::normalize(dir);

            glm::quat rot = rotate_from_to(vec3(0.0f, 0.0f, 1.0f), dir);

            vec3 mid = start + dir * (length * 0.5f);
            vec3 scale(0.1f, 0.1f, length);

            push_gizmo(Shapes::Cube, Transform(mid, rot, scale));
        }
    }
}

void visualize_path(const std::vector<glm::ivec3>& path) {
    for (size_t i = 0; i < path.size(); ++i) {
        vec3 pos = vec3(path[i]);

        // draw node
        push_gizmo(Shapes::Cube, Transform(pos, glm::vec3(0.05f)));

        // draw segment to next node
        if (i + 1 < path.size()) {
            vec3 start = pos;
            vec3 end = vec3(path[i + 1]);

            vec3 dir = end - start;
            float length = glm::length(dir);
            if (length <= 0.0001f)
                continue;

            dir = glm::normalize(dir);

            glm::quat rot = rotate_from_to(vec3(0.0f, 0.0f, 1.0f), dir);

            vec3 mid = start + dir * (length * 0.5f);
            vec3 scale(0.1f, 0.1f, length);

            push_gizmo(Shapes::Cube, Transform(mid, rot, scale));
        }
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
    n.speed = 10.0;
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

    if (npc.cant_find_route && npc.last_pathfounded < 0.0) {
        npc.last_pathfounded = 2.0; // 2 second cooldown to pathfind again
        npc.path_idx = 0;
        npc.cant_find_route = false;
        expand_navgraph(global_navGraph, worldBVHs, npc.position, npc.goal);
        npc.path = find_path(global_navGraph, npc.position, npc.goal);
    }
}

void update_npc(Npc& npc, float dt) {
    npc.old_position = npc.position;
    npc.velocity.y += GRAVITY.y * dt;
    npc.last_jumped -= dt;
    npc.last_pathfounded -= dt;

    if (npc.grounded) {
        npc.velocity.y = 0.0f;
    }

    // glm::vec3 dir = npc.goal - npc.position;
    // dir.y = 0.0f;
    /*
    if (glm::length2(dir) > 0.5) {
        npc.wishdir = glm::normalize(dir);
    } else {
        npc.wishdir = glm::vec3(0.0f);
    }
    */

    npc.velocity.x = npc.wishdir.x * npc.speed;
    npc.velocity.z = npc.wishdir.z * npc.speed;

    npc.position += npc.velocity * dt;
    float immediate_velocity = glm::length(npc.position - npc.old_position);

    for (const ivec3& point : npc.path) {
        push_gizmo(Shapes::Sphere, Transform(vec3(point), vec3(0.5)), 1, {1.0, 0.0, 0.0, 1.0});
    }

    if (!global_navGraph) {
        // global_navGraph = create_navgraph();
        // npc has no means of navigating
        return;
    }

    // npc is most likely stuck
    /*
    if (
        immediate_velocity < npc.speed * dt * 0.7 && 
        npc.last_pathfounded < 0.0 && 
        !npc.on_goal    
    ) {
        npc.cant_find_route = true;
        npc.last_pathfounded = 2.0; // 5 second cooldown to find another path
    }
    */

    npc.wishdir = vec3(0);

    if (!npc.path.empty() && npc.path_idx < npc.path.size()) {
        vec3 path = vec3(npc.path[npc.path_idx]);
        npc.wishdir = glm::normalize(path - npc.position);

        if (glm::distance(path, npc.position) < npc.collider.radius * 2.0) { // very close to goal node
            npc.path_idx++;
        }

        // jump if needed
        if (path.y - npc.collider.radius * 0.5 > npc.position.y && npc.last_jumped < 0.0) {
            printf("attempting to jump\n");
            npc.grounded = false;
            // push it a little bit off the ground to counter ground check
            // resetting velocity from jump
            npc.position.y += npc.collider.radius * 0.2; 
            npc.velocity.y = npc.jump_force;
            npc.last_jumped = 1.0; // 1 second cooldown on ujmp
        }
    }

    npc.on_goal = npc.path_idx >= npc.path.size();
}
