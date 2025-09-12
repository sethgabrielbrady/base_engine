#include "shared.h"


#define ISO_ANGLE_X x_cam // Angle in degrees for X rotation
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
#define FAR_PLANE 625.0f
#define ISO_CAM_DISTANCE 55.0f // Distance of the camera from the target
#define ISO_CAM_HEIGHT 40.0f   // Height of the camera
#define ISO_CLOSE_CAM_DISTANCE 25.0f // Distance of the camera from the target
#define ISO_CLOSE_CAM_HEIGHT 30.0f   // Height of the camera
#define GRAVITY 3.05f // Gravity effect on the player
#define SHOT_MAX 5

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
float walkAnimSpeed = 0.0f; // Store walk animation speed
float newTime = 0.0f;
float deltaTime = 0.0f;
float camRotationY = 0.0f;
float lastTime = 0.0f;
float x_cam = 0.0f;
float y_cam = 5.26f;

float speed = 0.0f;

float rot_x = 0.0f;

float reticuleScale = 0.125f;
float spinReticule = 0.0f;


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

bool lightsEnabled = false; // Flag to enable/disable lights
bool beamUp = true; // Flag to control the beam light's position



T3DVec3 camTarget = {{0, 0, -10}};
T3DVec3 playerPos = {{0, DEFAULT_PLAYER_Y, 0}};
T3DVec3 newPos = {{0, 0, 0}};

T3DVec3 camPos = {{0, 14.0f, 10.0f}};
T3DVec3 lightDirVec = {{0.0f, 0.10f, 0.0f}}; // this will eventually be part of the env struct or be updated by the environment struct

T3DVec3 reticulePos = {{15.0, 20.0, 0}};
T3DVec3 testSpherePos = {{0, 0, 0}}; // Position for the test sphere
// T3DVec3 crosshairPos = {{0, 0, 0}};

// T3DVec3 envPos = {{0.0f, 0.0f, 0.0f}};


// T3DVec3 newDir = {{0, 0, 0}};




//Vectors
T3DVec3 lockedShotDir = {{0, 0, 0}};
T3DVec3 moveDir = {{0, 0, 0}};
T3DVec3 lastPlayerPos = {{0, DEFAULT_PLAYER_Y, 0}};;
T3DVec3 cubePos = {{0, 0, 0}};
T3DVec3 shotDir = {{0, 0, 0}};
T3DVec3 crosshairPos = {{0, 0, 0}};

T3DVec3 normalizedShotDir;

sprite_t* bridge;
T3DViewport viewport;

//matrixes
T3DMat4FP *modelMatFP;
T3DMat4FP *envMatFP;
T3DMat4FP *cubeMatFP;
T3DMat4FP *reticuleMatFP;
T3DMat4FP *testSphereMatFP; // Matrix for the test sphere model
T3DMat4FP *crosshairMatFP; // Matrix for the test sphere model



//models
T3DModel *envModel;  // environment model
T3DModel *beam2;  // cube for  projectiles - plane with texture
T3DModel *reticule;  // cube for  projectiles - plane with texture
T3DModel *testSphere;  // cube for  projectiles - plane with texture
T3DModel *crosshair; // crosshair model



T3DModel *modelShadow; // shadow model
T3DModel *playerModel; // character model

//animation
T3DSkeleton skel; // skeleton for character model animation
T3DSkeleton skelBlend; // skeleton for blending animations
T3DAnim animIdle;
T3DAnim animWalk;
T3DAnim animRoll;
T3DAnim animJump;
T3DAnim animRun;



int sign(float value) {
  return (value > 0) - (value < 0); // Returns 1 for positive, -1 for negative, and 0 for zero
}

void update_timing() {
  newTime = get_time_s();
  deltaTime = newTime - lastTime;
  camRotationY = T3D_DEG_TO_RAD(x_cam);
  lastTime = newTime;
}

void update_camera() {
 // Isometric camera calculation
 float iso_angle_x_rad = T3D_DEG_TO_RAD(ISO_ANGLE_X);
 float iso_angle_y_rad = T3D_DEG_TO_RAD(ISO_ANGLE_Y);


 // camera should always look be behind the player
 if (!isPaused) {
    camTarget = playerPos; // Camera looks at player position
 } else {
  camTarget = testSpherePos; // Camera looks at test sphere position
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



//  if (isPaused) {
//    camPos.v[0] = pauseCameraX;
//    camPos.v[2] = pauseCameraY;
//    camTarget.v[0] = pauseCameraX;
//    camTarget.v[1] = pauseCameraY;
//  }

 t3d_viewport_set_perspective(&viewport, T3D_DEG_TO_RAD(DEFAULT_FOV), ASPECT_RATIO,
                               NEAR_PLANE, FAR_PLANE);

 t3d_viewport_look_at(&viewport, &camPos, &camTarget,
                        &(T3DVec3){{0, 1, 0}});
}

void update_crosshair_position() {
  // Calculate the forward direction of the camera
  T3DVec3 forwardVector = {
      .v[0] = camTarget.v[0] - camPos.v[0],
      .v[1] = camTarget.v[1] - camPos.v[1],
      .v[2] = camTarget.v[2] - camPos.v[2]
  };

  // Normalize the forward direction
  t3d_vec3_norm(&forwardVector);

//   // Set the crosshair position a fixed distance in front of the camera
  float crosshairDistance = 10.0f; // Distance in front of the camera
  crosshairPos.v[0] = camPos.v[0] + forwardVector.v[0] * crosshairDistance;
  crosshairPos.v[1] = camPos.v[1] + forwardVector.v[1] * crosshairDistance;
  crosshairPos.v[2] = camPos.v[2] + forwardVector.v[2] * crosshairDistance;

//   // The crosshair should always face the camera
//   // Use the camera's up vector for alignment
  T3DVec3 upVector = {{0.0f, 1.0f, 0.0f}}; // Assuming Y-axis is up

  // Update the crosshair matrix to face the camera
  void t3d_mat4_look_at(T3DMat4 *mat, const T3DVec3 *eye, const T3DVec3 *target, const T3DVec3 *up);
}

void update_animation() {
   //////// Animation ////////////
   animBlend = currSpeed;
   animBlend = clamp(animBlend, 0.0f, 1.0f);
   // Store walk animation speed for consistent blending
   walkAnimSpeed = animBlend + 0.15f; //Check if this is correct

    // Copy shotDir into normalizedShotDir
    // normalizedShotDir = shotDir;
    // // Normalize normalizedShotDir
    // t3d_vec3_norm(&normalizedShotDir);

    T3DVec3 forwardVector = {
        .v[0] = shotDir.v[0], // Forward direction based on shotDir
        .v[1] = 0.0f,         // No vertical offset for the forward vector
        .v[2] = shotDir.v[2]  // Forward direction based on shotDir
    };

    // Normalize the forward vector to ensure consistent offsets
    t3d_vec3_norm(&forwardVector);

    // Calculate the right vector (perpendicular to the forward direction)
    T3DVec3 rightVector = {
        .v[0] = -forwardVector.v[2], // Negate Z to get perpendicular X
        .v[1] = 0.0f,                // No vertical offset for the right vector
        .v[2] = forwardVector.v[0]   // Use X to get perpendicular Z
    };



  // Normalize the right vector to ensure consistent offsets
  t3d_vec3_norm(&rightVector);

   // move player...
   if (playerMove) {
    //  playerPos.v[0] += moveDir.v[0] * currSpeed;
    //  playerPos.v[2] += moveDir.v[2] * currSpeed;
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

  // crosshairPos.v[0] = playerPos.v[0];
  // crosshairPos.v[1] = playerPos.v[1]; // Slightly above the camera
  // crosshairPos.v[2] = playerPos.v[2] - 2.0f; // Slightly in front of the camera

   // when the shot is fired
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
    // This is where the beamLight is positioned
    // This seems very convoluted, but it works
    // I think the best solution would be to set a small plane witth a texture
    // and rotate it to face the camera, but this works for now
   // Set the cube's position relative to the player and fixed local axes
    cubePos.v[0] = playerPos.v[0] + (forwardVector.v[0] * 8) + (rightVector.v[0] * 4); // Forward and right offset
    cubePos.v[1] = playerPos.v[1] + PLAYER_TO_CUBE_Y + 2.0f;                           // Vertical offset
    cubePos.v[2] = playerPos.v[2] + (forwardVector.v[2] * 8) + (rightVector.v[2] * 4); // Forward and right offset
   }




  // Update the animation and modify the skeleton, this will however NOT
  // recalculate the matrices
  t3d_anim_update(&animIdle, deltaTime);
  t3d_anim_set_speed(&animWalk, walkAnimSpeed); // Use stored speed
  t3d_anim_update(&animWalk, deltaTime);

  if (speed >= 0.6f) {
    isRun = true;
    t3d_anim_update(&animRun, deltaTime);
  }

  if (isJump) {
    t3d_anim_update(&animJump, deltaTime);

    // playerPos.v[1] += 9.0f; // need to update jump to not animatio upwards
    if (!animJump.isPlaying) isJump = false;
  }

  if (isRoll) {
    // playerPos.v[1] = DEFAULT_PLAYER_Y - 5.0f;
    t3d_anim_update(&animRoll, deltaTime * 1.5f);
    if (!animRoll.isPlaying) isRoll = false;
  }

  // We now blend the walk animation with the idle/attack one
  t3d_skeleton_blend(&skel, &skel, &skelBlend, animBlend);

  // Now recalc. the matrices, this will cause any model referencing them to
  // use the new pose
  t3d_skeleton_update(&skel);


  // Update player matrix
  t3d_mat4fp_from_srt_euler(
      modelMatFP, (float[3]){0.19f, 0.19f, 0.19f},
      (float[3]){rotX, 0.0f, rotY}, // rotation
      playerPos.v);

  //update the crosshair position
  // crosshairPos.v[0] = playerPos.v[0] + (forwardVector.v[0] * 10) + (rightVector.v[0] * 4); // Forward and right offset
  // crosshairPos.v[1] = playerPos.v[1] + PLAYER_TO_CUBE_Y + 2.0f; // Vertical offset
  // crosshairPos.v[2] = playerPos.v[2] + (forwardVector.v[2] * 10) + (rightVector.v[2] * 4); // Forward and right offset
  // Update the crosshair matrix to face the camera
  // This will make the crosshair always face the camera
  // This is the crosshair matrix, it will be used to draw the crosshair
  // The crosshair should always face the camera

  // crosshairPos.v[0] = playerPos.v[0];
  // crosshairPos.v[1] = playerPos.v[1];
  // crosshairPos.v[2] = playerPos.v[2];

  float c_scale = 25.0f; // Scale for the crosshair
  t3d_mat4fp_from_srt_euler(crosshairMatFP, (float[3]){c_scale, c_scale, c_scale},
                             (float[3]){0.0f, 0.0f, 0.0f}, // rotation
                             crosshairPos.v);

  // shotsize and direction
  t3d_mat4fp_from_srt_euler(cubeMatFP, (float[3]){shotSize, shotSize, shotSize},
                            (float[3]){0.0f, spinShot, 0.0f}, // rotation
                            cubePos.v);

  spinReticule += deltaTime * 2.0f; // Spin the reticule
                            // reticule and direction
  t3d_mat4fp_from_srt_euler(reticuleMatFP, (float[3]){reticuleScale, reticuleScale, reticuleScale},
                             (float[3]){0.0f, 0.0f, 0.0f}, // rotation
                            reticulePos.v);


                              // reticule and direction


  // update_crosshair_position();

}





