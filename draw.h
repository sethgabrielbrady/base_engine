#ifndef DRAW_H
#define DRAW_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>
#include <math.h>
#include <rdpq.h>

#include "shared.h"
#include "input.h"
#include "lights.h"

extern rspq_block_t *dplPlayerCharacter;
extern rspq_block_t *dplMap;
extern rspq_block_t *dplCube;
extern rspq_block_t *testRet;

extern uint8_t colorAmbient[4];
extern uint8_t colorDir[4];

void init_models();
void draw_elements();

#endif
