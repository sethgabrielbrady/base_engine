#include "shared.h"

int shotCount = 0;
int lastZDir = 0;

float shotTimer = 0.0f;
float newAngle = 0.0f;
float rotY = 0.0f;
float currentHeight = DEFAULT_PLAYER_Y;
float shotSize = 0.025f;
float shotSizeMax = 0.06f;
float shotSizeMin = 0.025f;
float posX = 16;
float posY = 24;
float spinShot = 0.0f;
float new_height = 0.0f;
float jumpHeight = 0.0f;

float rotX = -1.50f;
float currSpeed = 0.0f;
float animBlend = 0.0f;
float shotSpeed = 0.0f;
float walkAnimSpeed = 0.0f;
float newTime = 0.0f;
float deltaTime = 0.0f;
float camRotationY = 0.0f;
float lastTime = 0.0f;
float x_cam = 0.0f;
float y_cam = 5.26f;

float speed = 0.0f;
float rot_x = 0.0f;
float reticuleScale = 0.125f;
float pauseCameraX = 0.0f;
float pauseCameraY = 0.0f;

bool isJump = false;
bool isRun = false;
bool shotFired = false;
bool aPress = false;
bool playerMove = true;
bool closeUp = false;
bool posSet = false;
bool isRoll = false;
bool shotHeld = false;
bool lightsEnabled = false;
bool beamUp = true;

T3DVec3 camTarget = {{0, 0, -10}};
T3DVec3 playerPos = {{0, DEFAULT_PLAYER_Y, 0}};
T3DVec3 newPos = {{0, 0, 0}};
T3DVec3 camPos = {{0, 14.0f, 10.0f}};
T3DVec3 lightDirVec = {{0.0f, 0.10f, 0.0f}};
T3DVec3 reticulePos = {{15.0, 20.0, 0}};

T3DVec3 lockedShotDir = {{0, 0, 0}};
T3DVec3 moveDir = {{0, 0, 0}};
T3DVec3 lastPlayerPos = {{0, DEFAULT_PLAYER_Y, 0}};
T3DVec3 cubePos = {{0, 0, 0}};
T3DVec3 shotDir = {{0, 0, 0}};

T3DViewport viewport;

// Matrices
T3DMat4FP *modelMatFP;
T3DMat4FP *envMatFP;
T3DMat4FP *cubeMatFP;
T3DMat4FP *reticuleMatFP;

// Models
T3DModel *envModel;
T3DModel *beam2;
T3DModel *reticule;
T3DModel *modelShadow;
T3DModel *playerModel;

// Animation
T3DSkeleton skel;
T3DSkeleton skelBlend;
T3DAnim animIdle;
T3DAnim animWalk;
T3DAnim animRoll;
T3DAnim animJump;
T3DAnim animRun;


void update_timing() {
  newTime = get_time_s();
  deltaTime = newTime - lastTime;
  camRotationY = T3D_DEG_TO_RAD(x_cam);
  lastTime = newTime;
}

void update_camera() {
  float iso_angle_x_rad = T3D_DEG_TO_RAD(ISO_ANGLE_X);
  float iso_angle_y_rad = T3D_DEG_TO_RAD(ISO_ANGLE_Y);

  if (!isPaused) {
    camTarget = playerPos;
  } else {
    camTarget = (T3DVec3){{0, 0, 0}};
  }

  if (!closeUp) {
    camTarget.v[1] = ISO_CAM_HEIGHT;
    camPos.v[0] = camTarget.v[0] + ISO_CAM_DISTANCE * cosf(iso_angle_y_rad) * sinf(iso_angle_x_rad);
    camPos.v[1] = camTarget.v[1] + DEFAULT_PLAYER_Y;
    camPos.v[2] = camTarget.v[2] + ISO_CAM_DISTANCE * cosf(iso_angle_y_rad) * cosf(iso_angle_x_rad);
  } else {
    camTarget.v[1] = ISO_CLOSE_CAM_HEIGHT;
    camPos.v[0] = camTarget.v[0] + ISO_CLOSE_CAM_DISTANCE * cosf(iso_angle_y_rad) * sinf(iso_angle_x_rad);
    camPos.v[1] = camTarget.v[1] + DEFAULT_PLAYER_Y;
    camPos.v[2] = camTarget.v[2] + ISO_CLOSE_CAM_DISTANCE * cosf(iso_angle_y_rad) * cosf(iso_angle_x_rad);
  }

  t3d_viewport_set_perspective(&viewport, T3D_DEG_TO_RAD(DEFAULT_FOV), ASPECT_RATIO,
                                NEAR_PLANE, FAR_PLANE);
  t3d_viewport_look_at(&viewport, &camPos, &camTarget,
                         &(T3DVec3){{0, 1, 0}});
}

void update_animation() {
  animBlend = currSpeed;
  animBlend = clamp(animBlend, 0.0f, 1.0f);
  walkAnimSpeed = animBlend + 0.15f;

  T3DVec3 forwardVector = {
      .v[0] = shotDir.v[0],
      .v[1] = 0.0f,
      .v[2] = shotDir.v[2]
  };
  t3d_vec3_norm(&forwardVector);

  T3DVec3 rightVector = {
      .v[0] = -forwardVector.v[2],
      .v[1] = 0.0f,
      .v[2] = forwardVector.v[0]
  };
  t3d_vec3_norm(&rightVector);

  // Move player
  if (playerMove) {
    newPos.v[0] += moveDir.v[0] * currSpeed;
    newPos.v[2] += moveDir.v[2] * currSpeed;
    playerPos = newPos;
    lastPlayerPos = playerPos;
    posSet = false;
  } else {
    playerPos = lastPlayerPos;
    if (!posSet) {
      rotY = 3.14593f;
      posSet = true;
      shotDir = newDir;
    }
  }

  // Shot movement
  if (shotFired) {
    shotTimer += deltaTime;
    if (shotTimer > SHOT_TIME_START && shotTimer < SHOT_TIME_END) {
      cubePos.v[0] += lockedShotDir.v[0] * SHOT_SPEED;
      cubePos.v[2] += lockedShotDir.v[2] * SHOT_SPEED;
      if (!playerMove) {
        if (cubePos.v[1] <= 50 && cubePos.v[1] >= 0) {
          lockedShotDir.v[1] = lockedShotDir.v[2] * -1;
          cubePos.v[1] += (lockedShotDir.v[2] * -1) * SHOT_SPEED / 5;
        }
      }
    } else {
      shotCount--;
      shotFired = false;
      shotSize = shotSizeMin;
    }
  } else {
    // Position beam light relative to player
    cubePos.v[0] = playerPos.v[0] + (forwardVector.v[0] * 8) + (rightVector.v[0] * 4);
    cubePos.v[1] = playerPos.v[1] + PLAYER_TO_CUBE_Y + 2.0f;
    cubePos.v[2] = playerPos.v[2] + (forwardVector.v[2] * 8) + (rightVector.v[2] * 4);
  }

  // Update animations
  t3d_anim_update(&animIdle, deltaTime);
  t3d_anim_set_speed(&animWalk, walkAnimSpeed);
  t3d_anim_update(&animWalk, deltaTime);

  if (speed >= 0.6f) {
    isRun = true;
    t3d_anim_update(&animRun, deltaTime);
  }

  if (isJump) {
    t3d_anim_update(&animJump, deltaTime);
    if (!animJump.isPlaying) isJump = false;
  }

  if (isRoll) {
    t3d_anim_update(&animRoll, deltaTime * 1.5f);
    if (!animRoll.isPlaying) isRoll = false;
  }

  t3d_skeleton_blend(&skel, &skel, &skelBlend, animBlend);
  t3d_skeleton_update(&skel);

  // Update player matrix
  t3d_mat4fp_from_srt_euler(
      modelMatFP, (float[3]){0.19f, 0.19f, 0.19f},
      (float[3]){rotX, 0.0f, rotY},
      playerPos.v);

  // Update shot matrix
  t3d_mat4fp_from_srt_euler(cubeMatFP, (float[3]){shotSize, shotSize, shotSize},
                            (float[3]){0.0f, spinShot, 0.0f},
                            cubePos.v);

  // Update reticule matrix
  t3d_mat4fp_from_srt_euler(reticuleMatFP, (float[3]){reticuleScale, reticuleScale, reticuleScale},
                             (float[3]){0.0f, 0.0f, 0.0f},
                            reticulePos.v);
}
