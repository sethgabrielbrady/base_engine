#ifndef AUDIO_H
#define AUDIO_H

#include <libdragon.h>
#include "shared.h"
#include "input.h"

extern wav64_t sfx_shot;
extern wav64_t sfx_step;
extern wav64_t sfx_charge;


extern float lastStepTime; // Tracks the last time sfx_step was played
extern float lastChargeTime; // Tracks the last time sfx_step was played
extern bool shotSoundPlayed; // Tracks if the shot sound has been played
extern bool isJumpSoundPlayed; // Tracks if the jump sound has been played


void initialize_audio();
void play_audio();
// extern stop_audio();


#endif //AUDIO_H