#ifndef COLLISION_H
#define COLLISION_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <float.h>
#include <math.h>

// Configuration constants
#define COLLISION_MAX_TRIANGLES 2048
#define COLLISION_EPSILON 0.0001f
#define COLLISION_MAX_DISTANCE 1000.0f

// Forward declarations
typedef struct CollisionMesh CollisionMesh;
typedef struct CollisionResult CollisionResult;

// Collision result structure
typedef struct CollisionResult {
    bool hit;
    float distance;
    T3DVec3 point;
    T3DVec3 normal;
    uint16_t triangle_id;
} CollisionResult;

// Optimized triangle structure for cache efficiency
typedef struct {
    T3DVec3 v0, v1, v2;    // Triangle vertices
    T3DVec3 normal;        // Pre-calculated normal
    T3DVec3 center;        // Triangle center for spatial sorting
    uint16_t id;           // Original triangle index
} CollisionTriangle;

// Collision mesh with spatial optimization
typedef struct CollisionMesh {
    CollisionTriangle* triangles;
    uint16_t triangle_count;
    T3DVec3 bounds_min, bounds_max;
    T3DVec3 bounds_center;
    float bounds_radius;
    bool is_valid;
} CollisionMesh;

// === HEADER DECLARATIONS ===

// Core collision mesh functions
CollisionMesh* collision_create_mesh(const T3DVec3* vertices, const uint16_t* indices,
                                   int vertex_count, int index_count);
void collision_destroy_mesh(CollisionMesh* mesh);
void collision_update_mesh_vertices(CollisionMesh* mesh, const T3DVec3* vertices,
                                   const uint16_t* indices, int vertex_count, int index_count);

// Primary collision functions
bool collision_ray_cast(const CollisionMesh* mesh, const T3DVec3* origin,
                       const T3DVec3* direction, float max_distance, CollisionResult* result);
bool collision_point_test(const CollisionMesh* mesh, const T3DVec3* point,
                         float max_distance, CollisionResult* result);
bool collision_sphere_test(const CollisionMesh* mesh, const T3DVec3* center,
                          float radius, CollisionResult* result);

// Utility functions
float collision_distance_to_point(const CollisionMesh* mesh, const T3DVec3* point);
bool collision_bounds_test(const CollisionMesh* mesh, const T3DVec3* point, float radius);

// === IMPLEMENTATION ===

// Helper function to initialize T3DVec3
static inline void set_vec3(T3DVec3* vec, float x, float y, float z) {
    vec->v[0] = x;
    vec->v[1] = y;
    vec->v[2] = z;
}

// Create optimized collision mesh from vertex data
CollisionMesh* collision_create_mesh(const T3DVec3* vertices, const uint16_t* indices,
                                   int vertex_count, int index_count) {
    if (!vertices || !indices || index_count == 0 || vertex_count == 0) return NULL;

    int triangle_count = index_count / 3;
    if (triangle_count > COLLISION_MAX_TRIANGLES) return NULL;

    CollisionMesh* mesh = malloc_uncached(sizeof(CollisionMesh));
    if (!mesh) return NULL;

    mesh->triangle_count = triangle_count;
    mesh->triangles = malloc_uncached(sizeof(CollisionTriangle) * mesh->triangle_count);

    if (!mesh->triangles) {
        free(mesh);
        return NULL;
    }

    // Initialize bounds
    set_vec3(&mesh->bounds_min, FLT_MAX, FLT_MAX, FLT_MAX);
    set_vec3(&mesh->bounds_max, -FLT_MAX, -FLT_MAX, -FLT_MAX);

    // Process triangles with optimizations
    for (int i = 0; i < mesh->triangle_count; i++) {
        CollisionTriangle* tri = &mesh->triangles[i];
        tri->id = i;

        // Get vertices from input arrays
        uint16_t i0 = indices[i * 3];
        uint16_t i1 = indices[i * 3 + 1];
        uint16_t i2 = indices[i * 3 + 2];

        // Bounds check for vertex indices
        if (i0 >= vertex_count || i1 >= vertex_count || i2 >= vertex_count) {
            collision_destroy_mesh(mesh);
            return NULL;
        }

        tri->v0 = vertices[i0];
        tri->v1 = vertices[i1];
        tri->v2 = vertices[i2];

        // Pre-calculate normal and center for performance
        T3DVec3 edge1, edge2;
        t3d_vec3_diff(&edge1, &tri->v1, &tri->v0);
        t3d_vec3_diff(&edge2, &tri->v2, &tri->v0);
        t3d_vec3_cross(&tri->normal, &edge1, &edge2);
        t3d_vec3_norm(&tri->normal);

        // Calculate triangle center
        tri->center.v[0] = (tri->v0.v[0] + tri->v1.v[0] + tri->v2.v[0]) / 3.0f;
        tri->center.v[1] = (tri->v0.v[1] + tri->v1.v[1] + tri->v2.v[1]) / 3.0f;
        tri->center.v[2] = (tri->v0.v[2] + tri->v1.v[2] + tri->v2.v[2]) / 3.0f;

        // Update bounds
        const T3DVec3* verts[] = {&tri->v0, &tri->v1, &tri->v2};
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                if (verts[j]->v[k] < mesh->bounds_min.v[k]) mesh->bounds_min.v[k] = verts[j]->v[k];
                if (verts[j]->v[k] > mesh->bounds_max.v[k]) mesh->bounds_max.v[k] = verts[j]->v[k];
            }
        }
    }

    // Calculate bounds center and radius
    t3d_vec3_add(&mesh->bounds_center, &mesh->bounds_min, &mesh->bounds_max);
    t3d_vec3_scale(&mesh->bounds_center, &mesh->bounds_center, 0.5f);

    T3DVec3 bounds_size;
    t3d_vec3_diff(&bounds_size, &mesh->bounds_max, &mesh->bounds_min);
    mesh->bounds_radius = t3d_vec3_len(&bounds_size) * 0.5f;

    mesh->is_valid = true;
    return mesh;
}

// Destroy collision mesh
void collision_destroy_mesh(CollisionMesh* mesh) {
    if (mesh) {
        if (mesh->triangles) free(mesh->triangles);
        free(mesh);
    }
}

// Fast ray-triangle intersection (optimized Möller-Trumbore)
static inline bool fast_ray_triangle_intersect(const T3DVec3* origin, const T3DVec3* dir,
                                              const CollisionTriangle* tri, float* t) {
    T3DVec3 edge1, edge2, h, s, q;
    float a, f, u, v;

    // Calculate edges
    t3d_vec3_diff(&edge1, &tri->v1, &tri->v0);
    t3d_vec3_diff(&edge2, &tri->v2, &tri->v0);

    t3d_vec3_cross(&h, dir, &edge2);
    a = t3d_vec3_dot(&edge1, &h);

    if (a > -COLLISION_EPSILON && a < COLLISION_EPSILON) return false;

    f = 1.0f / a;
    t3d_vec3_diff(&s, origin, &tri->v0);
    u = f * t3d_vec3_dot(&s, &h);

    if (u < 0.0f || u > 1.0f) return false;

    t3d_vec3_cross(&q, &s, &edge1);
    v = f * t3d_vec3_dot(dir, &q);

    if (v < 0.0f || u + v > 1.0f) return false;

    *t = f * t3d_vec3_dot(&edge2, &q);
    return *t > COLLISION_EPSILON;
}

// Optimized ray casting with early bounds rejection
bool collision_ray_cast(const CollisionMesh* mesh, const T3DVec3* origin,
                       const T3DVec3* direction, float max_distance, CollisionResult* result) {
    if (!mesh || !mesh->is_valid || !origin || !direction || !result) return false;

    // Initialize result
    result->hit = false;
    result->distance = max_distance;

    // Bounds check first (sphere test for quick rejection)
    T3DVec3 to_center;
    t3d_vec3_diff(&to_center, &mesh->bounds_center, origin);
    float dist_to_center = t3d_vec3_len(&to_center);

    if (dist_to_center > mesh->bounds_radius + max_distance) return false;

    // Test triangles
    float closest_t = max_distance;
    const CollisionTriangle* hit_triangle = NULL;

    for (int i = 0; i < mesh->triangle_count; i++) {
        const CollisionTriangle* tri = &mesh->triangles[i];
        float t;

        if (fast_ray_triangle_intersect(origin, direction, tri, &t)) {
            if (t < closest_t && t > 0.0f) {
                closest_t = t;
                hit_triangle = tri;
            }
        }
    }

    if (hit_triangle) {
        result->hit = true;
        result->distance = closest_t;
        result->triangle_id = hit_triangle->id;
        result->normal = hit_triangle->normal;

        // Calculate hit point
        T3DVec3 scaled_dir;
        t3d_vec3_scale(&scaled_dir, direction, closest_t);
        t3d_vec3_add(&result->point, origin, &scaled_dir);
    }

    return result->hit;
}

// Fast point-to-triangle distance calculation
static inline float point_triangle_distance_squared(const T3DVec3* point, const CollisionTriangle* tri) {
    // Simplified distance calculation for performance
    T3DVec3 to_center;
    t3d_vec3_diff(&to_center, point, &tri->center);
    return t3d_vec3_dot(&to_center, &to_center);
}

// Optimized point collision test
bool collision_point_test(const CollisionMesh* mesh, const T3DVec3* point,
                         float max_distance, CollisionResult* result) {
    if (!mesh || !mesh->is_valid || !point || !result) return false;

    result->hit = false;
    result->distance = max_distance;

    // Quick bounds check
    if (!collision_bounds_test(mesh, point, max_distance)) return false;

    float closest_dist_sq = max_distance * max_distance;
    const CollisionTriangle* closest_triangle = NULL;

    // Find closest triangle using squared distances for performance
    for (int i = 0; i < mesh->triangle_count; i++) {
        const CollisionTriangle* tri = &mesh->triangles[i];
        float dist_sq = point_triangle_distance_squared(point, tri);

        if (dist_sq < closest_dist_sq) {
            closest_dist_sq = dist_sq;
            closest_triangle = tri;
        }
    }

    if (closest_triangle) {
        result->hit = true;
        result->distance = sqrtf(closest_dist_sq);
        result->triangle_id = closest_triangle->id;
        result->normal = closest_triangle->normal;
        result->point = closest_triangle->center; // Approximation for performance
    }

    return result->hit;
}

// Sphere collision test
bool collision_sphere_test(const CollisionMesh* mesh, const T3DVec3* center,
                          float radius, CollisionResult* result) {
    if (!mesh || !mesh->is_valid || !center || !result) return false;

    result->hit = false;

    // Quick bounds check
    if (!collision_bounds_test(mesh, center, radius)) return false;

    // Test against triangles
    for (int i = 0; i < mesh->triangle_count; i++) {
        const CollisionTriangle* tri = &mesh->triangles[i];

        // Simplified sphere-triangle test
        T3DVec3 to_center;
        t3d_vec3_diff(&to_center, center, &tri->center);
        float dist_sq = t3d_vec3_dot(&to_center, &to_center);

        if (dist_sq <= radius * radius) {
            result->hit = true;
            result->triangle_id = tri->id;
            result->normal = tri->normal;
            result->point = tri->center;
            result->distance = sqrtf(dist_sq);
            return true;
        }
    }

    return false;
}

// Utility: Get minimum distance from point to mesh
float collision_distance_to_point(const CollisionMesh* mesh, const T3DVec3* point) {
    if (!mesh || !mesh->is_valid || !point) return FLT_MAX;

    CollisionResult result;
    if (collision_point_test(mesh, point, COLLISION_MAX_DISTANCE, &result)) {
        return result.distance;
    }
    return FLT_MAX;
}

// Utility: Fast bounds test
bool collision_bounds_test(const CollisionMesh* mesh, const T3DVec3* point, float radius) {
    if (!mesh || !mesh->is_valid || !point) return false;

    T3DVec3 to_point;
    t3d_vec3_diff(&to_point, point, &mesh->bounds_center);
    float distance = t3d_vec3_len(&to_point);

    return distance <= (mesh->bounds_radius + radius);
}

// Update mesh with new vertex data
void collision_update_mesh_vertices(CollisionMesh* mesh, const T3DVec3* vertices,
                                   const uint16_t* indices, int vertex_count, int index_count) {
    if (!mesh || !mesh->is_valid || !vertices || !indices) return;

    int triangle_count = index_count / 3;
    if (triangle_count != mesh->triangle_count) return; // Must match original size

    // Reset bounds
    set_vec3(&mesh->bounds_min, FLT_MAX, FLT_MAX, FLT_MAX);
    set_vec3(&mesh->bounds_max, -FLT_MAX, -FLT_MAX, -FLT_MAX);

    // Update vertices and recalculate data
    for (int i = 0; i < mesh->triangle_count; i++) {
        CollisionTriangle* tri = &mesh->triangles[i];

        uint16_t i0 = indices[i * 3];
        uint16_t i1 = indices[i * 3 + 1];
        uint16_t i2 = indices[i * 3 + 2];

        // Bounds check
        if (i0 >= vertex_count || i1 >= vertex_count || i2 >= vertex_count) continue;

        // Update vertices
        tri->v0 = vertices[i0];
        tri->v1 = vertices[i1];
        tri->v2 = vertices[i2];

        // Recalculate normal
        T3DVec3 edge1, edge2;
        t3d_vec3_diff(&edge1, &tri->v1, &tri->v0);
        t3d_vec3_diff(&edge2, &tri->v2, &tri->v0);
        t3d_vec3_cross(&tri->normal, &edge1, &edge2);
        t3d_vec3_norm(&tri->normal);

        // Update center
        tri->center.v[0] = (tri->v0.v[0] + tri->v1.v[0] + tri->v2.v[0]) / 3.0f;
        tri->center.v[1] = (tri->v0.v[1] + tri->v1.v[1] + tri->v2.v[1]) / 3.0f;
        tri->center.v[2] = (tri->v0.v[2] + tri->v1.v[2] + tri->v2.v[2]) / 3.0f;

        // Update bounds
        const T3DVec3* verts[] = {&tri->v0, &tri->v1, &tri->v2};
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                if (verts[j]->v[k] < mesh->bounds_min.v[k]) mesh->bounds_min.v[k] = verts[j]->v[k];
                if (verts[j]->v[k] > mesh->bounds_max.v[k]) mesh->bounds_max.v[k] = verts[j]->v[k];
            }
        }
    }

    // Recalculate bounds center and radius
    t3d_vec3_add(&mesh->bounds_center, &mesh->bounds_min, &mesh->bounds_max);
    t3d_vec3_scale(&mesh->bounds_center, &mesh->bounds_center, 0.5f);

    T3DVec3 bounds_size;
    t3d_vec3_diff(&bounds_size, &mesh->bounds_max, &mesh->bounds_min);
    mesh->bounds_radius = t3d_vec3_len(&bounds_size) * 0.5f;
}

#endif // COLLISION_H

// === USAGE EXAMPLE ===
/*
// In your main game file:

static CollisionMesh* level_collision = NULL;

// Example: Simple ground plane
void create_ground_collision() {
    T3DVec3 ground_vertices[4];
    set_vec3(&ground_vertices[0], -50.0f, 0.0f, -50.0f);
    set_vec3(&ground_vertices[1],  50.0f, 0.0f, -50.0f);
    set_vec3(&ground_vertices[2],  50.0f, 0.0f,  50.0f);
    set_vec3(&ground_vertices[3], -50.0f, 0.0f,  50.0f);

    uint16_t ground_indices[] = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };

    level_collision = collision_create_mesh(ground_vertices, ground_indices, 4, 6);
}

void game_update() {
    // Ray casting for ground detection
    T3DVec3 player_pos, down_dir;
    set_vec3(&player_pos, player.x, player.y + 1.0f, player.z);
    set_vec3(&down_dir, 0.0f, -1.0f, 0.0f);

    CollisionResult hit;
    if (collision_ray_cast(level_collision, &player_pos, &down_dir, 10.0f, &hit)) {
        player.ground_y = hit.point.v[1];
        player.on_ground = (player.y - hit.point.v[1]) < 0.1f;
    }

    // Sphere collision for player
    T3DVec3 player_center;
    set_vec3(&player_center, player.x, player.y + 1.0f, player.z);

    if (collision_sphere_test(level_collision, &player_center, 0.5f, &hit)) {
        // Player is colliding with ground
        player.y = hit.point.v[1] + 0.5f; // Push player up
    }
}

void game_cleanup() {
    collision_destroy_mesh(level_collision);
}
*/