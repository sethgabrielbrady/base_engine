#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>
#include <float.h>
#include <rdpq.h>
#include <math.h>

#include "input.h"
#include "shared.h"
#include "collision.h"
#include "draw.h"
#include "audio.h"
#include "stats.h"
#include "test_collision.h"

resolution_t custom_res = {
  .width = 320,
  .height = 240,
  .interlaced = false
};



// typedef struct {
//   bool isPaused;
//   // vec3 playerPos;
//   // vec3 lastPlayerPos;
//   float lastTime;
// } GameState;

// GameState gameState;

static CollisionMesh* level_collision = NULL;

// Helper function to set T3DVec3 values
static inline void set_vec3(T3DVec3* vec, float x, float y, float z) {
    vec->v[0] = x;
    vec->v[1] = y;
    vec->v[2] = z;
}

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


void game_init(void) {  // initialize various components

  // gameState.isPaused = false;
  // // gameState.playerPos = (vec3){0, 0, 0};
  // // gameState.lastPlayerPos = gameState.playerPos;
  // gameState.lastTime = get_time_s() - (1.0f / 60.0f);

  debug_init_isviewer(); // Initialize the debug system for isviewer
  debug_init_usblog(); // Initialize the debug system for USB logging
  asset_init_compression(2); // Initialize asset compression with level 2
  dfs_init(DFS_DEFAULT_LOCATION); // Initialize the DragonFS filesystem with default location
  display_init(custom_res, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE); // Initialize the display with custom resolution and settings
  rdpq_init(); // Initialize the RDP queue system
  joypad_init(); // Initialize the joypad system
  t3d_init((T3DInitParams){}); // Initialize the tiny3d library with default parameters
  rdpq_text_register_font( FONT_BUILTIN_DEBUG_MONO,rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO)); // Register a built-in debug font for text rendering
  viewport = t3d_viewport_create(); // Create a viewport with default values
  init_models(); // Initialize the 3D models and animations
  initialize_audio(); // Initialize the audio system

  // lastTime = get_time_s() - (1.0f / 60.0f);
}

void game_loop() {

  if (!isPaused) {
    lastPlayerPos = playerPos;
    update_timing();
    // check_bounds(); // Evironment collision checks
    update_animation();
    play_audio();
  }

  joypad_poll(); // pull the current state of the controller
  check_controller_state(); // check the controller state
  update_camera(); // Update the camera position and direction based on player input
  rdpq_attach(display_get(), display_get_zbuf());  //draw to screen

  draw_elements(); // Draw the 3D elements

  // Ray casting for ground detection
  T3DVec3 rayStart, down_dir;
  set_vec3(&rayStart, playerPos.v[0], playerPos.v[1] + 1.0f, playerPos.v[2]);
  set_vec3(&down_dir, 0.0f, -1.0f, 0.0f);

  CollisionResult hit;
  if (collision_ray_cast(level_collision, &rayStart, &down_dir, 10.0f, &hit)) {
      // Ground detected - you can store this info in global variables if needed
      float ground_y = hit.point.v[1];
      bool on_ground = (playerPos.v[1] - hit.point.v[1]) < 0.1f;

      // Adjust player height if needed
      if (playerPos.v[1] < ground_y + 0.15f) {
          playerPos.v[1] = ground_y + 0.15f;
      }
  }

  // Sphere collision for player
  T3DVec3 player_center;
  set_vec3(&player_center, playerPos.v[0], playerPos.v[1] + 1.0f, playerPos.v[2]);

  if (collision_sphere_test(level_collision, &player_center, 0.5f, &hit)) {
      // Player is colliding with ground
      playerPos.v[1] = hit.point.v[1] + 0.5f; // Push player up
  }

// End of collision detection example

  rdpq_mode_antialias(AA_NONE);

  updateReticule();
  // testCollision();

  rdpq_sync_pipe(); // Hardware crashes otherwise


  if (isPaused) {
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 120, 100, "Game Paused");
    // stats();
  }


  show_game_data(); // Display game data such as FPS and memory usage

  // rdpq_debug_start(); // Start the RDP debug block
  // rdpq_debug_log(true); // Log the debug information
  rdpq_detach_show();
}

int game_cleanup() { // --- Cleanup ---
  t3d_skeleton_destroy(&skel);
  t3d_skeleton_destroy(&skelBlend);
  t3d_anim_destroy(&animIdle);
  t3d_anim_destroy(&animWalk);
  t3d_anim_destroy(&animRun);
  t3d_anim_destroy(&animJump);
  t3d_anim_destroy(&animRoll);
  t3d_model_free(playerModel);
  t3d_model_free(envModel);
  t3d_model_free(modelShadow);
  t3d_destroy();
  audio_close();
  mixer_close();
  collision_destroy_mesh(level_collision);
  return 0;
}

int main() {
  game_init();

  while(1) { // Main game loop
    game_loop();
  };

  game_cleanup();
  return 0;
}
