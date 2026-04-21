#include "draw.h"
#include "collision_grid.h"

rspq_block_t *dplPlayerCharacter;
rspq_block_t *dplMap;
rspq_block_t *dplCube;
rspq_block_t *testRet;

static sprite_t *background;


void init_models() {

  modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
  envMatFP = malloc_uncached(sizeof(T3DMat4FP));
  cubeMatFP = malloc_uncached(sizeof(T3DMat4FP));
  reticuleMatFP = malloc_uncached(sizeof(T3DMat4FP));

  t3d_mat4fp_from_srt_euler(envMatFP, (float[3]){0.3f, 0.3f, 0.3f},
                             (float[3]){0, 0, 0}, (float[3]){0, 0, 0});

  // --- Load models ---
  envModel = t3d_model_load("rom:/vall3.t3dm");
  beam2 = t3d_model_load("rom:/beam5.t3dm");
  reticule = t3d_model_load("rom:/parasite4.t3dm");
  modelShadow = t3d_model_load("rom:/shadow2.t3dm");
  playerModel = t3d_model_load("rom:/samus_otest2.t3dm");
  skel = t3d_skeleton_create(playerModel);
  skelBlend = t3d_skeleton_clone(&skel, false);

  background = sprite_load("rom:/background.sprite");

  // Build collision grid from environment model (0.3 = envModel render scale)
  envCollision = collision_grid_create_from_model(envModel, 0.3f);

  // --- Set up animations ---
  animIdle = t3d_anim_create(playerModel, "idleShoot");
  t3d_anim_set_looping(&animIdle, true);
  t3d_anim_set_playing(&animIdle, true);
  t3d_anim_attach(&animIdle, &skel);

  animWalk = t3d_anim_create(playerModel, "walk");
  t3d_anim_attach(&animWalk, &skelBlend);

  animRun = t3d_anim_create(playerModel, "run");
  t3d_anim_attach(&animRun, &skelBlend);

  animRoll = t3d_anim_create(playerModel, "roll");
  t3d_anim_set_looping(&animRoll, false);
  t3d_anim_attach(&animRoll, &skelBlend);

  animJump = t3d_anim_create(playerModel, "jump2");
  t3d_anim_set_looping(&animJump, false);
  t3d_anim_attach(&animJump, &skelBlend);

  // --- Build display lists ---

  // Player model
  rspq_block_begin();
  t3d_matrix_push(modelMatFP);
  t3d_model_draw_skinned(playerModel, &skel);
  rdpq_set_prim_color(RGBA32(00, 00, 00, 120));
  t3d_model_draw(modelShadow);
  t3d_matrix_pop(1);
  dplPlayerCharacter = rspq_block_end();

  // Environment model
  rspq_block_begin();
  t3d_matrix_push(envMatFP);
  rdpq_set_prim_color(RGBA32(00, 00, 00, 120));
  t3d_model_draw(envModel);
  t3d_matrix_pop(1);
  dplMap = rspq_block_end();

  // Shot/projectile
  rspq_block_begin();
  t3d_matrix_push(cubeMatFP);
  rdpq_set_prim_color(RGBA32(00, 00, 00, 120));
  t3d_model_draw(beam2);
  t3d_matrix_pop(1);
  dplCube = rspq_block_end();

  // Reticule
  rspq_block_begin();
  t3d_matrix_push(reticuleMatFP);
  rdpq_set_prim_color(RGBA32(00, 00, 00, 120));
  t3d_model_draw(reticule);
  t3d_matrix_pop(1);
  testRet = rspq_block_end();

  if (lightsEnabled) {
    load_directional_light();
  }

  load_beam_point_light();
}


void draw_elements() {

  t3d_frame_start();
  t3d_screen_clear_color(RGBA32(0x20, 00, 0x20, 0xFF));
  t3d_screen_clear_depth();
  t3d_viewport_attach(&viewport);

  if (lightsEnabled) {
    draw_directional_light();
  }

  rspq_block_run(dplMap);

  if (!isPaused) {
    rspq_block_run(testRet);
  }

  if (shotTimer >= 0.01f && shotFired) {
    update_beam_point_light_position();
    draw_beam_point_light();
    rspq_block_run(dplCube);
  }

  if (shotHeld) {
    update_beam_point_light_position();
    draw_beam_point_light();
  }

  add_ambient_light();
  rspq_block_run(dplPlayerCharacter);
}
