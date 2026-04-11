#ifndef LIGHTS_H
#define LIGHTS_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>

void load_directional_light();
void draw_directional_light();
void load_beam_point_light();
void draw_beam_point_light();
void update_beam_point_light_position();
void add_ambient_light();

#endif
