#include "stats.h"


void check_memory_usage() {
  heap_stats_t stats;
  sys_get_heap_stats(&stats);

  rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 10, 50, "Heap Total: %d", stats.total);
  rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 10, 60, "Heap Used: %d", stats.used);
  rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 10, 70, "Heap Free: %d", stats.total - stats.used);
}

void show_game_data() {
  rdpq_text_printf(&(rdpq_textparms_t){}, FONT_BUILTIN_DEBUG_MONO, 10, 30, "FPS %f", display_get_fps());
  rdpq_text_printf(&(rdpq_textparms_t){}, FONT_BUILTIN_DEBUG_MONO, 10, 40, "Mem %d", get_memory_size());
  check_memory_usage();
}

void stats() {
  posY = 60;
  check_memory_usage();
}
