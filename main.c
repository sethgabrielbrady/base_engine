#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>
#include <rdpq.h>
#include <math.h>

#include "input.h"
#include "shared.h"
#include "collision.h"
#include "draw.h"
#include "audio.h"
#include "stats.h"

resolution_t custom_res = {
  .width = 320,
  .height = 240,
  .interlaced = false
};


void game_init(void) {
  debug_init_isviewer();
  debug_init_usblog();
  asset_init_compression(2);
  dfs_init(DFS_DEFAULT_LOCATION);
  display_init(custom_res, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);

  display_set_fps_limit(FRAME_RATE);
  rdpq_init();
  joypad_init();
  t3d_init((T3DInitParams){});
  rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));
  viewport = t3d_viewport_create();
  init_models();
  initialize_audio();
}

void game_loop() {

  if (!isPaused) {
    lastPlayerPos = playerPos;
    update_timing();
    update_animation();
    play_audio();
  }

  joypad_poll();
  check_controller_state();
  update_camera();
  rdpq_attach(display_get(), display_get_zbuf());

  draw_elements();

  rdpq_mode_antialias(AA_NONE);

  updateReticule();

  rdpq_sync_pipe();

  if (isPaused) {
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 120, 100, "Game Paused");
    stats();
  }

  show_game_data();

  rdpq_detach_show();
}

int game_cleanup() {
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
  return 0;
}

int main() {
  game_init();

  while (1) {
    game_loop();
  }

  game_cleanup();
  return 0;
}
