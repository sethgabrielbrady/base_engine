#ifndef TEST_COLLISION_H
#define TEST_COLLISION_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <float.h>

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

// === FUNCTION DECLARATIONS ===

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

#endif // TEST_COLLISION_H