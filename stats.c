
  #include "stats.h"

  void show_game_data() {
    rdpq_text_printf(&(rdpq_textparms_t){}, FONT_BUILTIN_DEBUG_MONO, 10, 30, "FPS %f", display_get_fps());
    rdpq_text_printf(&(rdpq_textparms_t){}, FONT_BUILTIN_DEBUG_MONO, 10, 40, "Mem %d", get_memory_size());
  };


  void stats() {
    posY = 50;
    posY += 10;
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "shotfired %d",
                                      shotFired);
    posY += 10;
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "reticule X %f",
                                      reticulePos.v[0]);
    posY += 10;
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "cube x %f",
      cubePos.v[0]);
  //                                     posY += 10;
  // rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "camposz%f",
  //                                     camPos.v[2]);
  // posY += 10;
  // rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "stick y%d",
  //                   joypad.stick_y);
  // posY += 10;
  // rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "planbe check %i",
  //   boundingPlaneCheck(-233.017899f, 220.436493f, -121.437119f, 156.0000f) );
  // posY += 10;
  // rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "x %f",
  //                   playerPos.v[0]);
  // posY += 10;
  // rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "y %f",
  // playerPos.v[1]);
  // posY += 10;
  // rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "z %f",
  // playerPos.v[2]);
  // posY += 10;
  // rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "height %f",
  //                   new_height);
  // posY += 10;
  // rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "y %f",
  //                   playerPos.v[1]);
  // posY += 10;
  // rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "z %f",
  //                   playerPos.v[2]);
  // posY += 10;

  };