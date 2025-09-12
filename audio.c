#include "audio.h"


wav64_t sfx_shot;
wav64_t sfx_step;
wav64_t sfx_charge;


float lastStepTime = 0.0f; // Tracks the last time sfx_step was played
float lastChargeTime = 0.0f; // Tracks the last time sfx_step was played
bool shotSoundPlayed = false; // Tracks if the shot sound has been played
bool isJumpSoundPlayed = false; // Tracks if the jump sound has been played

void initialize_audio() {
  // Initialize the audio system
  audio_init(44100, 4);
	mixer_init(8);
	// // mixer_ch_set_limits(6, 0, 128000, 0);
  wav64_open(&sfx_shot, "rom:/beam_wave_shot.wav64");
  wav64_open(&sfx_step, "rom:/samus_footstep.wav64");
  wav64_open(&sfx_charge, "rom:/beamcharge.wav64");
  wav64_set_loop(&sfx_shot, false);
  wav64_set_loop(&sfx_step, false);
  //
}

void play_audio() {
   // Audio handling
   if (shotFired && !shotSoundPlayed && !shotHeld) {
    wav64_play(&sfx_shot, 1); // Play the shot sound effect
    shotSoundPlayed = true;   // Mark the sound as played
  } else if (!shotFired) {
      shotSoundPlayed = false;  // Reset the flag when the shot is no longer fired
  }

  if (shotHeld && shotTimer == 0.0f && !shotFired) {
    float currentTime = get_time_s(); // Get the current time in seconds
    if (currentTime - lastChargeTime >= 0.22f) { // Check if 0.5 seconds have passed
        wav64_play(&sfx_charge, 0); // Play the step sound effect
        lastChargeTime = currentTime; // Update the last step time
    }
  }

  // if (isJump && !isJumpSoundPlayed) {
  //   wav64_play(&sfx_step, 1); // Play the jump sound effect
  //   isJumpSoundPlayed = true;   // Mark the sound as played
  // } else {
  //   isJumpSoundPlayed = false;  // Reset the flag when not jumping
  // }



  if (isRun && !isJump) {
    float currentTime = get_time_s(); // Get the current time in seconds
    if (currentTime - lastStepTime >= 0.22f) { // Check if 0.5 seconds have passed
        wav64_play(&sfx_step, 1); // Play the step sound effect
        lastStepTime = currentTime; // Update the last step time
    }
  }


  mixer_try_play();

}
