#ifndef COLLISION_GRID_H
#define COLLISION_GRID_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <stdbool.h>

// Triangle classification by surface normal
#define TRI_TYPE_FLOOR   0  // normal.y > 0.5  (walkable surfaces, slopes up to ~60 deg)
#define TRI_TYPE_WALL    1  // |normal.y| <= 0.5
#define TRI_TYPE_CEILING 2  // normal.y < -0.5

// Grid configuration
#define GRID_CELL_SIZE     32.0f   // World units per grid cell
#define MAX_COLLISION_TRIS 2048    // Max triangles the grid can hold

// A single collision triangle in world space
typedef struct {
    T3DVec3 v0, v1, v2;
    T3DVec3 normal;
    uint8_t type;  // TRI_TYPE_FLOOR, WALL, or CEILING
} CollisionTri;

// A grid cell references a range in the index array
typedef struct {
    uint16_t startIdx;  // Offset into the triIndices array
    uint16_t count;     // Number of triangle indices in this cell
} GridCell;

// Result of a floor height query
typedef struct {
    bool found;
    float height;
    T3DVec3 normal;
} FloorResult;

// Result of a wall push-out query
typedef struct {
    bool pushed;
    T3DVec3 newPos;
} WallResult;

// The spatial grid for fast collision lookups
typedef struct {
    CollisionTri *tris;
    uint16_t triCount;

    uint16_t *triIndices;   // Flat array: cells reference ranges into this
    uint32_t totalIndices;

    GridCell *cells;
    int gridWidth, gridHeight;
    float originX, originZ;  // World-space origin of grid corner
    float cellSize;

    T3DVec3 boundsMin, boundsMax;
} CollisionGrid;


// Create a collision grid by extracting triangles from a T3DModel.
// modelScale is the uniform scale applied when rendering (e.g. 0.3f).
CollisionGrid* collision_grid_create_from_model(const T3DModel *model, float modelScale);

// Query the floor height at world position (x, z).
// Returns true if ground was found, writing height into result.
FloorResult collision_grid_get_floor(const CollisionGrid *grid, float x, float z);

// Push the player out of any walls within radius of pos.
// Returns adjusted position.
WallResult collision_grid_push_walls(const CollisionGrid *grid, T3DVec3 pos, float radius);

// Free all memory used by the grid.
void collision_grid_destroy(CollisionGrid *grid);

#endif
