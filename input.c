#include "input.h"

joypad_inputs_t joypad;
joypad_buttons_t btn;
joypad_buttons_t btnHeld;

float rotatedStickX = 0.0f;
float rotatedStickY = 0.0f;

bool isPaused = false;

T3DVec3 newDir = {{0.0f, 0.0f, 0.0f}};

void check_controller_state(void) {
  joypad = joypad_get_inputs(JOYPAD_PORT_1);
  btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
  btnHeld = joypad_get_buttons_held(JOYPAD_PORT_1);

  rotatedStickX = joypad.stick_x * cosf(camRotationY) - joypad.stick_y * sinf(camRotationY);
  rotatedStickY = joypad.stick_x * sinf(camRotationY) + joypad.stick_y * cosf(camRotationY);

  newDir = (T3DVec3){{rotatedStickX * 0.5f, 0, -rotatedStickY * 0.5f}};
  speed = sqrtf(t3d_vec3_len2(&newDir));

  if (btnHeld.r) {
    closeUp = true;
    playerMove = false;
    isRun = false;
    walkAnimSpeed = 0.0f;
    isJump = false;
    isRoll = false;
  } else {
    closeUp = false;
    playerMove = true;
    camTarget = playerPos;
  }

  // Camera rotation with stick
  if (!closeUp) {
    if (joypad.stick_x > 70) {
      x_cam -= 1.5f;
    } else if (joypad.stick_x < -70) {
      x_cam += 1.5f;
    }
  } else {
    if (joypad.stick_x > 40) {
      x_cam -= 1.5f;
    } else if (joypad.stick_x < -40) {
      x_cam += 1.5f;
    }
  }

  // Camera rotation with C-buttons
  if (!isPaused) {
    if (btnHeld.c_left) {
      x_cam -= 2.5f;
    } else if (btnHeld.c_right) {
      x_cam += 2.5f;
    }
  } else {
    if (btnHeld.z) {
      if (btnHeld.c_left) {
        x_cam -= 2.5f;
      } else if (btnHeld.c_right) {
        x_cam += 2.5f;
      }
    }
  }

  if (!isPaused) {

    // Player movement
    isRun = speed >= 0.6f;

    if (speed > 0.15f) {
      newDir.v[0] /= speed;
      newDir.v[2] /= speed;
      moveDir = newDir;

      if (!shotFired) {
        shotDir = newDir;
      }

      newAngle = atan2f(moveDir.v[0], moveDir.v[2]);
      rotY = playerMove ? t3d_lerp_angle(0.0f, ROTATION_SPEED, newAngle)
                        : t3d_lerp_angle(0.0f, ROTATION_SPEED, newAngle);

      currSpeed = t3d_lerp(currSpeed, speed * PLAYER_MOVE_SPEED, 0.15f);
    } else {
      currSpeed *= 0.8f;
    }

    currSpeed = isRun ? PLAYER_RUN_SPEED : t3d_lerp(currSpeed, speed * PLAYER_MOVE_SPEED, 0.15f);

    if (newDir.v[2] != 0.0f) {
      lastZDir = signum((int)newDir.v[2]);
    } else {
      lastZDir = -1;
    }

    // Jump
    if (btn.a && (!isJump && playerMove)) {
      aPress = true;
      isJump = true;
      jumpHeight += 0.10f;
      t3d_anim_set_playing(&animJump, true);
      t3d_anim_set_time(&animJump, 0.0f);
    } else {
      aPress = false;
    }

    if (isJump) {
      currSpeed = PLAYER_JUMP_SPEED;
      if (!isRun) {
        moveDir.v[0] = 0.0f;
        moveDir.v[1] += JUMP_VELOCITY;
        moveDir.v[2] = 0.0f;
      }
    }

    // Roll
    if (btn.c_down && (!isRoll && playerMove)) {
      isRoll = true;
      t3d_anim_set_playing(&animRoll, true);
      t3d_anim_set_time(&animRoll, 0.50f);
    }

    // Shot
    if (btnHeld.b && !shotFired) {
      shotHeld = true;
      shotSize = clamp(shotSize + SHOT_SIZE_INCREMENT, shotSizeMin, shotSizeMax);
      shotTimer = 0.0f;
    } else if (shotHeld && !shotFired) {
      shotCount++;
      shotFired = true;
      shotTimer = 0.0f;
      shotHeld = false;
      lockedShotDir = shotDir;
    }

    if (btn.b && !shotFired && !shotHeld && shotCount < SHOT_MAX) {
      shotSize = shotSizeMin;
      shotCount++;
      shotFired = true;
      shotTimer = 0.0f;
      lockedShotDir = shotDir;
    }

    if (shotFired && shotSize > shotSizeMin) {
      shotSize -= SHOT_SIZE_INCREMENT;
      if (shotSize < shotSizeMin) {
        shotSize = shotSizeMin;
      }
    }
  }

  if (btn.start) {
    isPaused = !isPaused;
  }
}
