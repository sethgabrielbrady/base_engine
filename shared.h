#ifndef SHARED_H
#define SHARED_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <rdpq.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

#include <math.h>

#include "input.h"



// --- Added for Isometric Camera ---
#define ISO_ANGLE_X x_cam  // Angle in degrees for X rotation
#define ISO_ANGLE_Y y_cam // Angle in degrees for Y rotation (arctan(1/sqrt(2)))
#define DEFAULT_PLAYER_Y 0.15f

#define SHOT_TIME_START 0.0f
#define SHOT_TIME_END 0.422f
#define PLAYER_TO_CUBE_Y 19.50f
#define SHOT_SPEED 20.0f
#define PLAYER_MOVE_SPEED 0.15f
#define PLAYER_RUN_SPEED 1.5f
#define PLAYER_JUMP_SPEED 2.5f
#define JUMP_VELOCITY 0.38f
#define ANIM_BLEND_DIVISOR 0.51f
#define ROTATION_SPEED 1.0f
#define SHOT_SIZE_INCREMENT 0.003f
#define DEFAULT_FOV 90.0f
#define ASPECT_RATIO 1.33f
#define NEAR_PLANE 10.0f
#define FAR_PLANE 625.0f // distance for camera
// #define DEFAULT_PLAYER_Y 0.15f

#define ISO_CAM_DISTANCE 55.0f // Distance of the camera from the target
#define ISO_CAM_HEIGHT 40.0f   // Height of the camera
#define ISO_CLOSE_CAM_DISTANCE 25.0f // Distance of the camera from the target
#define ISO_CLOSE_CAM_HEIGHT 30.0f   // Height of the camera
#define GRAVITY 3.05f // Gravity effect on the player
#define SHOT_MAX 5


extern int shotCount;
extern int lastZDir;

extern float shotTimer;
extern float newAngle;
extern float rotY;
extern float currentHeight;
extern float shotSize;
extern float shotSizeMax;
extern float shotSizeMin;
extern float posX;
extern float posY;
extern float spinShot;
extern float new_height;
extern float jumpHeight;
extern float speed;


extern float rotX;
extern float currSpeed;
extern float animBlend;
extern float shotSpeed;
extern float walkAnimSpeed; // Store walk animation speed
extern float newTime;
extern float deltaTime;
extern float camRotationY;
extern float lastTime;
extern float x_cam;
extern float y_cam;
extern float rot_x;

extern float pauseCameraX;
extern float pauseCameraY;

extern float reticuleScale;

extern bool isJump;
extern bool isRun;
extern bool shotFired;
extern bool aPress;
extern bool playerMove;
extern bool closeUp;
extern bool posSet;
extern bool isRoll;
extern bool shotHeld;
extern bool lightsEnabled; // Flag to enable/disable lights
extern bool beamUp; // Flag to control beam light strength fluctuation


//Vectors
extern T3DVec3 lockedShotDir;
extern T3DVec3 moveDir;
extern T3DVec3 lastPlayerPos;
extern T3DVec3 cubePos;
extern T3DVec3 shotDir;
extern T3DVec3 camTarget;
extern T3DVec3 playerPos;
extern T3DVec3 newPos;

extern T3DVec3 camPos;
extern T3DVec3 reticulePos;
extern T3DVec3 testSpherePos;
extern T3DVec3 crosshairPos;




extern T3DVec3 lightDirVec;
// extern T3DVec3 newDir;
// extern T3DVec3 envPos;

extern sprite_t* bridge;
extern T3DViewport viewport;

//matrixes
extern T3DMat4FP *modelMatFP;
extern T3DMat4FP *envMatFP;
extern T3DMat4FP *cubeMatFP;
extern T3DMat4FP *reticuleMatFP;
extern T3DMat4FP *testSphereMatFP;
extern T3DMat4FP *crosshairMatFP;
extern T3DMat4FP *fulgorerMatFP; // Matrix for the Fulgore model



//models
extern T3DMat4FP *envMatFP;

extern T3DModel *envModel;  // environment model
extern T3DModel *beam2;  // cube for  projectiles - plane with texture
extern T3DModel *reticule;  // cube for  projectiles - plane with texture
extern T3DModel *testSphere; // test sphere model
extern T3DModel *crosshair; // crosshair model
extern T3DModel *fulgore; // Fulgore model

extern T3DModel *modelShadow; // shadow model
extern T3DModel *playerModel; // character model

//animation
extern T3DSkeleton skel; // skeleton for character model animation
extern T3DSkeleton skelBlend; // skeleton for blending animations
extern T3DAnim animIdle;
extern T3DAnim animWalk;
extern T3DAnim animRoll;
extern T3DAnim animJump;
extern T3DAnim animRun;






// Function declarations and definitions
static inline float get_time_s() {
  return (float)((double)get_ticks_us() / 1000000.0);
}

static inline int signum(int num) {
  return (num > 0) - (num < 0);
}

static inline float clamp(float val, float minVal, float maxVal) {
  return fmaxf(fminf(val, maxVal), minVal);
}

void update_timing();

void update_camera();

void update_animation();

#endif
