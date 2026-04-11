#ifndef INPUT_H
#define INPUT_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

#include "shared.h"

extern joypad_inputs_t joypad;
extern joypad_buttons_t btn;
extern joypad_buttons_t btnHeld;

extern T3DVec3 newDir;
extern float rotatedStickX;
extern float rotatedStickY;
extern float speed;
extern bool isPaused;

void check_controller_state();

#endif
