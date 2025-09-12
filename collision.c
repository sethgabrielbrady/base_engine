#include "collision.h"




float randX = 0.0f;
float randZ = 0.0f;
float randY = 0.0f;

T3DVec3 playerLastPos = {{0.0f, 0.15f, 0.0f}}; // Last player position
bool beamHit = false;


typedef struct {
  T3DVec3 *points; // Array of vectors (collision points)
  size_t length;   // Number of points in the array
  float collisionRadius; // Radius for collision detection
} Collider;

// can probably rewrite to take t3dVec
bool checkCollisionPoints(Collider *collider, T3DVec3 playerPos) {
  if (!collider || !collider->points || collider->length < 3) {
      return false; // Invalid collider or not enough points to form a polygon
  }

  bool inside = false;
  float px = playerPos.v[0];
  float pz = playerPos.v[2];
  float buffer = 0.0f; // Remove buffer for testing

  for (size_t i = 0, j = collider->length - 1; i < collider->length; j = i++) {
      T3DVec3 pointI = collider->points[i];
      T3DVec3 pointJ = collider->points[j];

      float xi = pointI.v[0], zi = pointI.v[2];
      float xj = pointJ.v[0], zj = pointJ.v[2];

      // Debug edge coordinates
      debugf("Edge (%f, %f) -> (%f, %f)\n", xi, zi, xj, zj);

      // Check if the ray intersects the edge of the polygon
      bool intersect = ((zi > pz) != (zj > pz)) &&
                       (px < (xj - xi) * (pz - zi) / (zj - zi) + xi);

      if (intersect) {
          debugf("Intersection detected!\n");
          inside = !inside; // Toggle the inside state
      }
  }

  return inside; // Return true if the point is inside the polygon
}



void testCollision() {
  // Define an array of collision points (forming a polygon)
  T3DVec3 collisionPoints[] = {
      {{256.0f, 0.0f, -74.0f}},
      {{336.0f, 0.0f, -36.0f}},
      {{376.0f, 0.0f, -106.0f}},
      {{302.0f, 0.0f, -180.0f}}
  };

  // Create a Collider struct
  Collider collider = {
      .points = collisionPoints,
      .length = sizeof(collisionPoints) / sizeof(collisionPoints[0])
  };

  if (playerMove == !checkCollisionPoints(&collider, playerPos) ) {
    playerLastPos = playerPos; // Update last player position if no collision
  } else if (playerMove == checkCollisionPoints(&collider, playerPos)) {
    playerPos = playerLastPos; // Reset player position to last position if collision detected
  }
  //if the player is in the collision area, set playerMove to false

}





void check_bounds(void) {
  //turn these intp separate functions
  float getHeight(float xz1, float xz2, float y1, float y2,float axis ) {
    float m = (y2 - y1) / (xz2 - xz1);
    float b = y1 - (m * xz1);
    return (m * axis) + b;
  }

  bool boundingPlaneCheck (float x1, float x2, float z1, float z2) {
    return ((playerPos.v[0] > x1 && playerPos.v[0] < x2) &&
            (playerPos.v[2] > z1 && playerPos.v[2] < z2));
  }

  playerMove = boundingPlaneCheck(-233.017899f, 220.436493f, -121.437119f, 156.0000f);

  if (playerPos.v[0] > -235.0000f && playerPos.v[0] < -232.00000f)  {
    playerPos.v[0] = -231.9999f;
    playerMove = true;
  }
  if (playerPos.v[0] > 217.0000f && playerPos.v[0] < 222.00000f)  {
    playerPos.v[0] = 216.9999f;
    playerMove = true;
  }
  if (playerPos.v[2] < 158.0000f && playerPos.v[2] > 154.00000f)  {
    playerPos.v[2] = 153.9999f;
    playerMove = true;
  }
  if (playerPos.v[2] > -123.0000f && playerPos.v[2] < -118.00000f)  {
    playerPos.v[2] = -117.9999f;
    playerMove = true;
  }


  if (playerPos.v[0] > -235.0000f && playerPos.v[0] < -232.00000f)  {
    playerPos.v[0] = -231.9999f;
    playerMove = true;
  }
  if (playerPos.v[0] > 217.0000f && playerPos.v[0] < 222.00000f)  {
    playerPos.v[0] = 216.9999f;
    playerMove = true;
  }
  if (playerPos.v[2] < 158.0000f && playerPos.v[2] > 154.00000f)  {
    playerPos.v[2] = 153.9999f;
    playerMove = true;
  }
  if (playerPos.v[2] > -123.0000f && playerPos.v[2] < -118.00000f)  {
    playerPos.v[2] = -117.9999f;
    playerMove = true;
  }

  bool pitCheck(float playerY, float x1) {
    return (playerPos.v[0] > x1);
  }

  if (playerPos.v[1] < -3.000f) {
    playerMove = pitCheck(playerPos.v[1], 74.434044f);
    if (playerPos.v[0] < 75.999f && playerPos.v[0] > 72.000f)  {
      playerPos.v[0] = 76.000f;
      playerMove = true;
    }
  }

  if (playerPos.v[0] <= -82.00f && playerPos.v[0] >= -235.00f) {
    new_height = getHeight(-82.00f, -235.00f, 0.0f, 28.00f, playerPos.v[0]);
  }else if (playerPos.v[2] <= 156.00f && playerPos.v[2] >= -120.00f && playerPos.v[0] >= 72.00f && playerPos.v[0] <= 225.00f) {
    new_height = getHeight(156.00f, -120.00f, 0.0f, -21.00f, playerPos.v[2]);
  } else if (new_height > DEFAULT_PLAYER_Y) {
    new_height -= GRAVITY; // falling speed;
  } else {
    new_height = DEFAULT_PLAYER_Y;
  }

  playerPos.v[1] = new_height;
}






void updateReticule() {

  //Reticule posuition and collision Check
  void updateReticulePos() {
    // get the positive or negative value of randX
    // float PosNegX = (randX < 0.0f) ? -randX : randX; // Get the absolute value of randX
    // float PosNegZ = (randZ < 0.0f) ? -randZ : randZ; // Get the absolute value of randZ

    reticulePos.v[1] = 20.0f;
    // reset reticule position to random position

    // move the reticule to a random position slowly
    if (!beamHit) {
      if (reticulePos.v[0] != randX) {
        if (randX <= 0.0f ) {
            reticulePos.v[0] -= 1;
        } else if (randX >= 0.1f ) {
            reticulePos.v[0] += 1;
        }
      }
    }



    if (reticulePos.v[2] != randZ) {
      if (randZ < 0.10f && reticulePos.v[2] != randZ) {
          reticulePos.v[2] -= 1;
      } else if (randZ >= 0.0f && reticulePos.v[2] != randZ) {
          reticulePos.v[2] += 1;
      }
    }


    if (reticulePos.v[1] != randY) {
      if (randY < 0.10f && reticulePos.v[1] != randY) {
          reticulePos.v[1] -= 1;
      } else if (randY >= 0.0f && reticulePos.v[1] != randY) {
          reticulePos.v[1] += 1;
      }
    }
  }

  void updateReticuleScale () {
    if (reticuleScale > 0.01f && beamHit) {
      reticuleScale -= 0.1f; // Decrease the scale
    } else {
      reticuleScale = 0.125f; // Ensure it doesn't go below a minimum value
      beamHit = false; // Reset beam hit state
    }
  }

  if (beamHit) {
    updateReticuleScale();
    reticulePos.v[0] = (float)(rand() % 100 - 50);
    reticulePos.v[2] = (float)(rand() % 100 - 50);
    reticulePos.v[1] = (float)(rand() % 100 - 50); // Random X between -50 and 50

  }

  //collision check for the reticule and the cube
  if (shotFired) {
    // Calculate the squared distance between the reticule and the cube
    float deltaX = reticulePos.v[0] - cubePos.v[0];
    float deltaY = reticulePos.v[1] - cubePos.v[1];
    float deltaZ = reticulePos.v[2] - cubePos.v[2];
    float distanceSquared = deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
    // Define a collision threshold (radius squared)
    float collisionThresholdSquared = 105.0f;

    // Check if the distance is within the collision threshold
    if (distanceSquared <= collisionThresholdSquared) {
        cubePos.v[0] = playerPos.v[0] + shotDir.v[0] * 5;
        cubePos.v[1] = playerPos.v[1] + PLAYER_TO_CUBE_Y;
        cubePos.v[2] = playerPos.v[2] + shotDir.v[2] * 8;
        // Reset shot
        shotCount--;
        shotSize = shotSizeMin;
        beamHit = true;

        randX = (float)(rand() % 100 - 50); // Random X between -50 and 50
        randY = (float)(rand() % 100 - 50); // Random X between -50 and 50
        randZ = (float)(rand() % 100 - 50); // Random Z between -50 and 50

        shotFired = false;
    }
  }

  if (reticulePos.v[0] != randX || reticulePos.v[2] != randZ) {
    updateReticulePos();
  } else if (reticulePos.v[0] == randX && reticulePos.v[2] == randZ) {
    // Reset reticule position to random position
    randX = (float)(rand() % 100 - 50); // Random X between -50 and 50
    randY = (float)(rand() % 100 - 50); // Random X between -50 and 50
    randZ = (float)(rand() % 100 - 50); // Random Z between -50 and 50
    updateReticulePos();
  }

  if (reticulePos.v[0] > 200 || reticulePos.v[0] < -200 ||
    reticulePos.v[2] > 200 || reticulePos.v[2] < -200 ) {
    // Reset reticule position to random position
    randX = 0.0f;
    randZ = 0.0f;
    reticulePos.v[0] = 0.0f;
    reticulePos.v[2] = 0.0f;
    // updateReticulePos();
  }
}




void add_test_sphere() {

  t3d_mat4fp_from_srt_euler(testSphereMatFP, (float[3]){0.025f, 0.025f, 0.025f},
                             (float[3]){0.0f, 0.0f, 0.0f}, // rotation
                              testSpherePos.v);


 // Add a sphere to the scene for testing purposes
  // This sphere will be used to test the reticule position and collision

  //use reticulePos as the test sphere position
  //Move testsphere with c-buttons
  if (!btnHeld.z) {
    if (btnHeld.c_left) {
      testSpherePos.v[0] -= 2.0f;
    } else if (btnHeld.c_right) {
      testSpherePos.v[0] += 2.0f;
    }
  }
  if (btnHeld.z) {
    if (btnHeld.c_up) {
      testSpherePos.v[1] += 2.0f;
    } else if (btnHeld.c_down) {
      testSpherePos.v[1] -= 2.0f;
    }
  } else {
    if (btnHeld.c_up) {
      testSpherePos.v[2] += 2.0f;
    } else if (btnHeld.c_down) {
      testSpherePos.v[2] -= 2.0f;
    }
  }


  rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 10, 10, "Test Sphere Position: X: %.2f, Y: %.2f, Z: %.2f",
    testSpherePos.v[0], testSpherePos.v[1], testSpherePos.v[2]);

}

