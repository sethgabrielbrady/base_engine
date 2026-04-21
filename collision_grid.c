#include "collision_grid.h"
#include <stdlib.h>
#include <math.h>
#include <float.h>


// ---- Internal helpers ----

// Classify triangle by its normal
static uint8_t classify_triangle(const T3DVec3 *normal) {
    if (normal->v[1] > 0.5f) return TRI_TYPE_FLOOR;
    if (normal->v[1] < -0.5f) return TRI_TYPE_CEILING;
    return TRI_TYPE_WALL;
}

// Compute triangle normal from vertices (not normalized yet)
static void compute_normal(const T3DVec3 *v0, const T3DVec3 *v1, const T3DVec3 *v2, T3DVec3 *out) {
    T3DVec3 e1, e2;
    e1.v[0] = v1->v[0] - v0->v[0];
    e1.v[1] = v1->v[1] - v0->v[1];
    e1.v[2] = v1->v[2] - v0->v[2];
    e2.v[0] = v2->v[0] - v0->v[0];
    e2.v[1] = v2->v[1] - v0->v[1];
    e2.v[2] = v2->v[2] - v0->v[2];
    out->v[0] = e1.v[1] * e2.v[2] - e1.v[2] * e2.v[1];
    out->v[1] = e1.v[2] * e2.v[0] - e1.v[0] * e2.v[2];
    out->v[2] = e1.v[0] * e2.v[1] - e1.v[1] * e2.v[0];
    t3d_vec3_norm(out);
}

// Check if point (px, pz) is inside triangle projected onto XZ plane using barycentric coords
static bool point_in_tri_xz(float px, float pz,
                             const T3DVec3 *a, const T3DVec3 *b, const T3DVec3 *c) {
    float ax = c->v[0] - a->v[0], az = c->v[2] - a->v[2];
    float bx = b->v[0] - a->v[0], bz = b->v[2] - a->v[2];
    float cx = px - a->v[0],      cz = pz - a->v[2];

    float d00 = ax * ax + az * az;
    float d01 = ax * bx + az * bz;
    float d02 = ax * cx + az * cz;
    float d11 = bx * bx + bz * bz;
    float d12 = bx * cx + bz * cz;

    float denom = d00 * d11 - d01 * d01;
    if (fabsf(denom) < 1e-8f) return false;

    float inv = 1.0f / denom;
    float u = (d11 * d02 - d01 * d12) * inv;
    float v = (d00 * d12 - d01 * d02) * inv;

    return (u >= 0.0f) && (v >= 0.0f) && (u + v <= 1.0f);
}

// Get the height (Y) of a plane defined by triangle at position (x, z)
static float tri_height_at(const CollisionTri *tri, float x, float z) {
    const T3DVec3 *n = &tri->normal;
    if (fabsf(n->v[1]) < 1e-6f) return tri->v0.v[1];
    return tri->v0.v[1] - (n->v[0] * (x - tri->v0.v[0]) + n->v[2] * (z - tri->v0.v[2])) / n->v[1];
}

// Get grid cell index, returns -1 if out of bounds
static int grid_cell_idx(const CollisionGrid *grid, float x, float z) {
    int cx = (int)((x - grid->originX) / grid->cellSize);
    int cz = (int)((z - grid->originZ) / grid->cellSize);
    if (cx < 0 || cx >= grid->gridWidth || cz < 0 || cz >= grid->gridHeight) return -1;
    return cz * grid->gridWidth + cx;
}

// Closest point on line segment AB to point P
static void closest_point_on_segment(const T3DVec3 *a, const T3DVec3 *b, const T3DVec3 *p, T3DVec3 *out) {
    T3DVec3 ab, ap;
    for (int i = 0; i < 3; i++) {
        ab.v[i] = b->v[i] - a->v[i];
        ap.v[i] = p->v[i] - a->v[i];
    }
    float dot_ab = ab.v[0]*ab.v[0] + ab.v[1]*ab.v[1] + ab.v[2]*ab.v[2];
    if (dot_ab < 1e-8f) { *out = *a; return; }
    float t = (ap.v[0]*ab.v[0] + ap.v[1]*ab.v[1] + ap.v[2]*ab.v[2]) / dot_ab;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    for (int i = 0; i < 3; i++) out->v[i] = a->v[i] + t * ab.v[i];
}

// Closest point on triangle to a point (for wall collision)
static void closest_point_on_triangle(const CollisionTri *tri, const T3DVec3 *p, T3DVec3 *out) {
    // First try projecting onto the triangle plane
    const T3DVec3 *n = &tri->normal;
    float dist = (p->v[0] - tri->v0.v[0]) * n->v[0]
               + (p->v[1] - tri->v0.v[1]) * n->v[1]
               + (p->v[2] - tri->v0.v[2]) * n->v[2];

    T3DVec3 proj;
    for (int i = 0; i < 3; i++) proj.v[i] = p->v[i] - dist * n->v[i];

    // Check if projected point is inside triangle
    if (point_in_tri_xz(proj.v[0], proj.v[2], &tri->v0, &tri->v1, &tri->v2)) {
        *out = proj;
        return;
    }

    // Otherwise find closest point on the three edges
    T3DVec3 c0, c1, c2;
    closest_point_on_segment(&tri->v0, &tri->v1, p, &c0);
    closest_point_on_segment(&tri->v1, &tri->v2, p, &c1);
    closest_point_on_segment(&tri->v2, &tri->v0, p, &c2);

    float d0 = 0, d1 = 0, d2 = 0;
    for (int i = 0; i < 3; i++) {
        float diff;
        diff = p->v[i] - c0.v[i]; d0 += diff * diff;
        diff = p->v[i] - c1.v[i]; d1 += diff * diff;
        diff = p->v[i] - c2.v[i]; d2 += diff * diff;
    }

    if (d0 <= d1 && d0 <= d2) *out = c0;
    else if (d1 <= d2) *out = c1;
    else *out = c2;
}


// ---- Triangle extraction from T3DModel ----

// Extract all triangles from a T3DModel into a CollisionTri array.
// Returns the number of triangles extracted.
static int extract_triangles(const T3DModel *model, float scale, CollisionTri *outTris, int maxTris) {
    int triCount = 0;

    // We simulate the vertex cache to resolve indices
    // The RSP vertex cache is 70 entries; parts load vertices at specific offsets
    T3DVec3 vcache[T3D_VERTEX_CACHE_SIZE];

    T3DModelIter it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
    while (t3d_model_iter_next(&it)) {
        T3DObject *obj = it.object;

        for (uint16_t p = 0; p < obj->numParts; p++) {
            T3DObjectPart *part = &obj->parts[p];

            // Load this part's vertices into our simulated cache
            // part->vert points into the global vertex buffer
            // part->vertLoadCount is how many vertices to load
            // part->vertDestOffset is where in the cache they go
            for (int v = 0; v < part->vertLoadCount; v++) {
                int16_t *pos = t3d_vertbuffer_get_pos(part->vert, v);
                int cacheIdx = part->vertDestOffset + v;
                if (cacheIdx >= T3D_VERTEX_CACHE_SIZE) continue;

                vcache[cacheIdx].v[0] = (float)pos[0] * scale;
                vcache[cacheIdx].v[1] = (float)pos[1] * scale;
                vcache[cacheIdx].v[2] = (float)pos[2] * scale;
            }

            // Extract regular indexed triangles
            for (int i = 0; i + 2 < part->numIndices; i += 3) {
                if (triCount >= maxTris) goto done;

                int i0 = part->indices[i];
                int i1 = part->indices[i + 1];
                int i2 = part->indices[i + 2];

                if (i0 >= T3D_VERTEX_CACHE_SIZE ||
                    i1 >= T3D_VERTEX_CACHE_SIZE ||
                    i2 >= T3D_VERTEX_CACHE_SIZE) continue;

                CollisionTri *tri = &outTris[triCount];
                tri->v0 = vcache[i0];
                tri->v1 = vcache[i1];
                tri->v2 = vcache[i2];
                compute_normal(&tri->v0, &tri->v1, &tri->v2, &tri->normal);
                tri->type = classify_triangle(&tri->normal);
                triCount++;
            }

            // Extract sequential (unindexed) triangles
            for (int s = 0; s < part->idxSeqCount; s++) {
                if (triCount >= maxTris) goto done;

                int base = part->idxSeqBase + s * 3;
                if (base + 2 >= T3D_VERTEX_CACHE_SIZE) continue;

                CollisionTri *tri = &outTris[triCount];
                tri->v0 = vcache[base];
                tri->v1 = vcache[base + 1];
                tri->v2 = vcache[base + 2];
                compute_normal(&tri->v0, &tri->v1, &tri->v2, &tri->normal);
                tri->type = classify_triangle(&tri->normal);
                triCount++;
            }
        }
    }

done:
    debugf("Collision: extracted %d triangles from model\n", triCount);
    return triCount;
}


// ---- Grid construction ----

// Count how many cells a triangle overlaps (for pre-allocation)
static void tri_cell_range(const CollisionGrid *grid, const CollisionTri *tri,
                           int *minCX, int *maxCX, int *minCZ, int *maxCZ) {
    float minX = fminf(fminf(tri->v0.v[0], tri->v1.v[0]), tri->v2.v[0]);
    float maxX = fmaxf(fmaxf(tri->v0.v[0], tri->v1.v[0]), tri->v2.v[0]);
    float minZ = fminf(fminf(tri->v0.v[2], tri->v1.v[2]), tri->v2.v[2]);
    float maxZ = fmaxf(fmaxf(tri->v0.v[2], tri->v1.v[2]), tri->v2.v[2]);

    *minCX = (int)((minX - grid->originX) / grid->cellSize);
    *maxCX = (int)((maxX - grid->originX) / grid->cellSize);
    *minCZ = (int)((minZ - grid->originZ) / grid->cellSize);
    *maxCZ = (int)((maxZ - grid->originZ) / grid->cellSize);

    if (*minCX < 0) *minCX = 0;
    if (*maxCX >= grid->gridWidth) *maxCX = grid->gridWidth - 1;
    if (*minCZ < 0) *minCZ = 0;
    if (*maxCZ >= grid->gridHeight) *maxCZ = grid->gridHeight - 1;
}


CollisionGrid* collision_grid_create_from_model(const T3DModel *model, float modelScale) {
    if (!model) return NULL;

    // Extract triangles into temp buffer
    CollisionTri *tempTris = malloc(sizeof(CollisionTri) * MAX_COLLISION_TRIS);
    if (!tempTris) return NULL;

    int triCount = extract_triangles(model, modelScale, tempTris, MAX_COLLISION_TRIS);
    if (triCount == 0) {
        free(tempTris);
        return NULL;
    }

    CollisionGrid *grid = malloc(sizeof(CollisionGrid));
    if (!grid) { free(tempTris); return NULL; }

    // Copy triangles to permanent storage (exact size)
    grid->tris = malloc(sizeof(CollisionTri) * triCount);
    if (!grid->tris) { free(grid); free(tempTris); return NULL; }
    memcpy(grid->tris, tempTris, sizeof(CollisionTri) * triCount);
    grid->triCount = triCount;
    free(tempTris);

    // Compute world bounds from triangles
    grid->boundsMin = (T3DVec3){{FLT_MAX, FLT_MAX, FLT_MAX}};
    grid->boundsMax = (T3DVec3){{-FLT_MAX, -FLT_MAX, -FLT_MAX}};
    for (int i = 0; i < triCount; i++) {
        const T3DVec3 *verts[] = {&grid->tris[i].v0, &grid->tris[i].v1, &grid->tris[i].v2};
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                if (verts[j]->v[k] < grid->boundsMin.v[k]) grid->boundsMin.v[k] = verts[j]->v[k];
                if (verts[j]->v[k] > grid->boundsMax.v[k]) grid->boundsMax.v[k] = verts[j]->v[k];
            }
        }
    }

    // Set up grid dimensions
    grid->cellSize = GRID_CELL_SIZE;
    grid->originX = grid->boundsMin.v[0] - grid->cellSize;
    grid->originZ = grid->boundsMin.v[2] - grid->cellSize;
    float extentX = grid->boundsMax.v[0] - grid->originX + grid->cellSize;
    float extentZ = grid->boundsMax.v[2] - grid->originZ + grid->cellSize;
    grid->gridWidth = (int)(extentX / grid->cellSize) + 1;
    grid->gridHeight = (int)(extentZ / grid->cellSize) + 1;

    int cellCount = grid->gridWidth * grid->gridHeight;
    grid->cells = malloc(sizeof(GridCell) * cellCount);
    memset(grid->cells, 0, sizeof(GridCell) * cellCount);

    debugf("Collision grid: %d x %d cells, %d tris, bounds (%.1f,%.1f)-(%.1f,%.1f)\n",
           grid->gridWidth, grid->gridHeight, triCount,
           grid->boundsMin.v[0], grid->boundsMin.v[2],
           grid->boundsMax.v[0], grid->boundsMax.v[2]);

    // Pass 1: count triangles per cell
    for (int i = 0; i < triCount; i++) {
        int minCX, maxCX, minCZ, maxCZ;
        tri_cell_range(grid, &grid->tris[i], &minCX, &maxCX, &minCZ, &maxCZ);
        for (int cz = minCZ; cz <= maxCZ; cz++) {
            for (int cx = minCX; cx <= maxCX; cx++) {
                grid->cells[cz * grid->gridWidth + cx].count++;
            }
        }
    }

    // Compute offsets (prefix sum)
    uint32_t total = 0;
    for (int i = 0; i < cellCount; i++) {
        grid->cells[i].startIdx = total;
        total += grid->cells[i].count;
        grid->cells[i].count = 0;  // Reset for pass 2
    }
    grid->totalIndices = total;

    // Allocate index array
    grid->triIndices = malloc(sizeof(uint16_t) * total);

    // Pass 2: fill triangle indices per cell
    for (int i = 0; i < triCount; i++) {
        int minCX, maxCX, minCZ, maxCZ;
        tri_cell_range(grid, &grid->tris[i], &minCX, &maxCX, &minCZ, &maxCZ);
        for (int cz = minCZ; cz <= maxCZ; cz++) {
            for (int cx = minCX; cx <= maxCX; cx++) {
                int cellIdx = cz * grid->gridWidth + cx;
                GridCell *cell = &grid->cells[cellIdx];
                grid->triIndices[cell->startIdx + cell->count] = i;
                cell->count++;
            }
        }
    }

    debugf("Collision grid built: %lu total cell-triangle entries\n", (unsigned long)total);
    return grid;
}


// ---- Queries ----

FloorResult collision_grid_get_floor(const CollisionGrid *grid, float x, float z) {
    FloorResult result = {.found = false, .height = -FLT_MAX};

    if (!grid) return result;

    int cellIdx = grid_cell_idx(grid, x, z);
    if (cellIdx < 0) return result;

    const GridCell *cell = &grid->cells[cellIdx];

    for (uint16_t i = 0; i < cell->count; i++) {
        const CollisionTri *tri = &grid->tris[grid->triIndices[cell->startIdx + i]];
        if (tri->type != TRI_TYPE_FLOOR) continue;

        if (point_in_tri_xz(x, z, &tri->v0, &tri->v1, &tri->v2)) {
            float h = tri_height_at(tri, x, z);
            if (h > result.height) {
                result.found = true;
                result.height = h;
                result.normal = tri->normal;
            }
        }
    }

    return result;
}


WallResult collision_grid_push_walls(const CollisionGrid *grid, T3DVec3 pos, float radius) {
    WallResult result = {.pushed = false, .newPos = pos};

    if (!grid) return result;

    // Check the 3x3 neighborhood of cells around the player
    int centerCX = (int)((pos.v[0] - grid->originX) / grid->cellSize);
    int centerCZ = (int)((pos.v[2] - grid->originZ) / grid->cellSize);

    for (int dz = -1; dz <= 1; dz++) {
        for (int dx = -1; dx <= 1; dx++) {
            int cx = centerCX + dx;
            int cz = centerCZ + dz;
            if (cx < 0 || cx >= grid->gridWidth || cz < 0 || cz >= grid->gridHeight) continue;

            int cellIdx = cz * grid->gridWidth + cx;
            const GridCell *cell = &grid->cells[cellIdx];

            for (uint16_t i = 0; i < cell->count; i++) {
                const CollisionTri *tri = &grid->tris[grid->triIndices[cell->startIdx + i]];
                if (tri->type != TRI_TYPE_WALL) continue;

                // Find closest point on triangle to player
                T3DVec3 closest;
                closest_point_on_triangle(tri, &result.newPos, &closest);

                // Check distance
                float dx2 = result.newPos.v[0] - closest.v[0];
                float dy = result.newPos.v[1] - closest.v[1];
                float dz2 = result.newPos.v[2] - closest.v[2];
                float distSq = dx2 * dx2 + dy * dy + dz2 * dz2;

                if (distSq < radius * radius && distSq > 1e-8f) {
                    float dist = sqrtf(distSq);
                    float penetration = radius - dist;

                    // Push out along the direction from closest point to player
                    float invDist = 1.0f / dist;
                    result.newPos.v[0] += dx2 * invDist * penetration;
                    result.newPos.v[1] += dy * invDist * penetration;
                    result.newPos.v[2] += dz2 * invDist * penetration;
                    result.pushed = true;
                }
            }
        }
    }

    return result;
}


void collision_grid_destroy(CollisionGrid *grid) {
    if (!grid) return;
    if (grid->tris) free(grid->tris);
    if (grid->triIndices) free(grid->triIndices);
    if (grid->cells) free(grid->cells);
    free(grid);
}
