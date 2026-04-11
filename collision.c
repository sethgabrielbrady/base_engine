#include "collision.h"

float randX = 0.0f;
float randZ = 0.0f;
float randY = 0.0f;

bool beamHit = false;


void updateReticule() {

  void updateReticulePos() {
    reticulePos.v[1] = 20.0f;

    if (!beamHit) {
      if (reticulePos.v[0] != randX) {
        if (randX <= 0.0f) {
          reticulePos.v[0] -= 1;
        } else if (randX >= 0.1f) {
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

  void updateReticuleScale() {
    if (reticuleScale > 0.01f && beamHit) {
      reticuleScale -= 0.01f;
    } else {
      reticuleScale = 0.125f;
      beamHit = false;
    }
  }

  if (beamHit) {
    updateReticuleScale();
  }

  // If player collides with reticule, move to new random position
  if (playerPos.v[0] == reticulePos.v[0] && playerPos.v[1] == reticulePos.v[1] && playerPos.v[2] == reticulePos.v[2]) {
    reticulePos.v[0] = (float)(rand() % 100 - 50);
    reticulePos.v[1] = (float)(rand() % 100 - 50);
    reticulePos.v[2] = (float)(rand() % 100 - 50);
  }

  // Collision check for the reticule and the shot
  if (shotFired) {
    float deltaX = reticulePos.v[0] - cubePos.v[0];
    float deltaY = reticulePos.v[1] - cubePos.v[1];
    float deltaZ = reticulePos.v[2] - cubePos.v[2];
    float distanceSquared = deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
    float collisionThresholdSquared = 105.0f;

    if (distanceSquared <= collisionThresholdSquared) {
      cubePos.v[0] = playerPos.v[0] + shotDir.v[0] * 5;
      cubePos.v[1] = playerPos.v[1] + PLAYER_TO_CUBE_Y;
      cubePos.v[2] = playerPos.v[2] + shotDir.v[2] * 8;
      shotCount--;
      shotSize = shotSizeMin;
      beamHit = true;

      randX = (float)(rand() % 100 - 50);
      randY = (float)(rand() % 100 - 50);
      randZ = (float)(rand() % 100 - 50);

      shotFired = false;
    }
  }

  if (reticulePos.v[0] != randX || reticulePos.v[2] != randZ) {
    updateReticulePos();
  } else if (reticulePos.v[0] == randX && reticulePos.v[2] == randZ) {
    randX = (float)(rand() % 100 - 50);
    randY = (float)(rand() % 100 - 50);
    randZ = (float)(rand() % 100 - 50);
    updateReticulePos();
  }

  if (reticulePos.v[0] > 200 || reticulePos.v[0] < -200 ||
      reticulePos.v[2] > 200 || reticulePos.v[2] < -200) {
    randX = 0.0f;
    randZ = 0.0f;
    reticulePos.v[0] = 0.0f;
    reticulePos.v[2] = 0.0f;
  }
}
