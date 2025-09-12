#include "draw.h"

rspq_block_t *dplPlayerCharacter;
rspq_block_t *dplMap;
rspq_block_t *dplCube;
rspq_block_t *testRet;
rspq_block_t *dplTestSphere;
rspq_block_t *dplCrosshair;
rspq_block_t *dplFulgore;
// sprite_t* crosshair;

static sprite_t *background;






void init_models() {

  modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
  envMatFP = malloc_uncached(sizeof(T3DMat4FP));
  cubeMatFP = malloc_uncached(sizeof(T3DMat4FP));
  reticuleMatFP = malloc_uncached(sizeof(T3DMat4FP));
  testSphereMatFP = malloc_uncached(sizeof(T3DMat4FP));
  crosshairMatFP = malloc_uncached(sizeof(T3DMat4FP));


  t3d_mat4fp_from_srt_euler(envMatFP, (float[3]){0.3f, 0.3f, 0.3f},
                             (float[3]){0, 0, 0}, (float[3]){0, 0, 0});

   // --- Load models ---

  envModel = t3d_model_load("rom:/vall3.t3dm");  // environment model
  beam2 = t3d_model_load("rom:/beam5.t3dm");  // cube for  projectiles - plane with texture
  reticule = t3d_model_load("rom:/beam4.t3dm");  // cube for  projectiles - plane with
  testSphere = t3d_model_load("rom:/testcube.t3dm"); // test sphere model
  crosshair = t3d_model_load("rom:/beam4.t3dm"); // crosshair model


  modelShadow = t3d_model_load("rom:/shadow2.t3dm"); // shadow model
  playerModel = t3d_model_load("rom:/samus_otest2.t3dm"); // character model
  skel = t3d_skeleton_create(playerModel); // skeleton for character model animation
  skelBlend = t3d_skeleton_clone(&skel, false); // skeleton for blending animations

  // bridge = sprite_load("rom:/bridge.sprite");
  background = sprite_load("rom:/background.sprite"); // crosshair sprite


  // --- Set up the models animations ---
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

  // --- Set up the 3D models ---

  // player model
  rspq_block_begin();
  t3d_matrix_push(modelMatFP);
  t3d_model_draw_skinned(playerModel, &skel);
  rdpq_set_prim_color(RGBA32(00, 00, 00, 120));
  t3d_model_draw(modelShadow);
  t3d_matrix_pop(1);
  dplPlayerCharacter = rspq_block_end();

  // environment model
  rspq_block_begin();
  t3d_matrix_push(envMatFP);
  rdpq_set_prim_color(RGBA32(00, 00, 00, 120));
  t3d_model_draw(envModel);
  t3d_matrix_pop(1);
  dplMap = rspq_block_end();

  // shot
  rspq_block_begin();
  t3d_matrix_push(cubeMatFP);
  rdpq_set_prim_color(RGBA32(00, 00, 00, 120));
  t3d_model_draw(beam2);
  t3d_matrix_pop(1);
  dplCube = rspq_block_end();


  // reticule test
  rspq_block_begin();
  t3d_matrix_push(reticuleMatFP);
  rdpq_set_prim_color(RGBA32(00, 00, 00, 120));
  t3d_model_draw(reticule);
  t3d_matrix_pop(1);
  testRet = rspq_block_end();

  //testSphere test
  rspq_block_begin();
  t3d_matrix_push(testSphereMatFP);
  rdpq_set_prim_color(RGBA32(00, 00, 00, 120));
  t3d_model_draw(testSphere);
  t3d_matrix_pop(1);
  dplTestSphere = rspq_block_end();

  // //crosshair test
  rspq_block_begin();
  t3d_matrix_push(crosshairMatFP);
  rdpq_set_prim_color(RGBA32(00, 00, 00, 120));
  t3d_model_draw(crosshair);
  t3d_matrix_pop(1);
  dplCrosshair = rspq_block_end();


  if (lightsEnabled) {
    // Initialize the directional light
    load_directional_light();
  }

  // load_point_light(); // Initialize the point light
  load_beam_point_light(); // Initialize the beam point light
}


void add_background(void) {
  rdpq_sprite_blit(background, 160, 120, &(rdpq_blitparms_t){
      // .s0 = 0, //Extract correct sprite from sheet
      // .t0 = 0,
      //Set sprite center to bottom-center
      .cx = 160,
      .cy = 120,
      // .width = 320, //Extract correct width from sheet
      // .height = 240,
      .scale_x = 1.2,
  });
  rdpq_sync_tile();
  rdpq_sync_pipe(); // Ensure the sprite is rendered before proceeding
}

void draw_elements() {

  // rdpq_attach(display_get(), display_get_zbuf()); // Attach the display and depth buffer


  t3d_frame_start();
  t3d_screen_clear_color(RGBA32(0x20, 00, 0x20, 0xFF)); // dark blue bg
  t3d_screen_clear_depth(); // clear depth buffer
  t3d_viewport_attach(&viewport);


  // rdpq_set_mode_standard();
  // rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
  // // rdpq_mode_combiner(RDPQ_COMBINER_TEX);
  // add_background(); // Draw the background sprite

  // rdpq_sync_pipe(); // Ensure the viewport is set before drawing

  // ======== Draw (3D) ======== //

  if (lightsEnabled) {
    draw_directional_light(); // Add the directional light before drawing the models
  }


  rspq_block_run(dplMap);
  rspq_block_run(dplCrosshair); // Set the reticule matrix for drawing


  if (isPaused) {
    rspq_block_run(dplTestSphere);
    add_test_sphere(); // Update the test sphere position
  } else {
    rspq_block_run(testRet);
    // Draw the test sphere
  }



  if (shotTimer >= 0.01f && shotFired) {
    // Update and draw the beam as a point light
    update_beam_point_light_position();
    draw_beam_point_light();
    rspq_block_run(dplCube);
  }

  if (shotHeld){
    update_beam_point_light_position();
    draw_beam_point_light();
    // rspq_block_run(dplCube);
  }




  add_ambient_light();

  rspq_block_run(dplPlayerCharacter);





  // rdpq_sprite_blit(crosshair, 160, 120, &(rdpq_blitparms_t){
  //   .cx = 160, // Center of the sprite
  //   .cy = 120,
  //   .scale_x = 3.0f,
  //   .scale_y = 3.0f,
  // });
  // rdpq_set_mode_standard();
  // rdpq_mode_combiner(RDPQ_COMBINER_TEX);
  // rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);

  // rdpq_sync_tile();
  // rspq_block_run(dplCrosshair);

  // if (closeUp) { //move to state machine
  //   posY = 130;
  //   rspq_block_run(dplCrosshair);
  //   rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY +10, "x %f",
  //     crosshairPos.v[0]);
  //   rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY +20, "y %f",
  //       crosshairPos.v[1]);
  //   rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY +30, "z %f",
  //         crosshairPos.v[2]);
  //   // display the crosshair rotation
  // }


  // if (closeUp) {
  //   // Draw the crosshair at the center of the screen
  //   isRun = false; // Disable run animation when close up
  //   rdpq_sprite_blit(crosshair, 160, 120, &(rdpq_blitparms_t){
  //     .cx = 160, // Center of the sprite
  //     .cy = 120,
  //     .scale_x = 3.0f,
  //     .scale_y = 3.0f,
  //   });
  //   rdpq_sync_tile();
  // }


  // draw_point_light(); // Add the point light before drawing the models
  // rdpq_detach_show();
  // rdpq_detach();

  // rdpq_sync_tile();
  //rdp_sync();
}