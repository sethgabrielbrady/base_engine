
#include "lights.h"
#include "shared.h"
#include <rspq_profile.h>


uint8_t colorDir[4] = {0xff, 0xff, 0xff, 0xff};
uint8_t pointLightColorAmbient[4] = {20, 20, 20, 0xFF};
uint8_t lightColorAmbient[4] = {0, 0, 0, 0xFF};

uint8_t colorAmbient[4] = {0x55, 0x55, 0x55, 0xFF};
// uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
// uint8_t colorAmbient[4] = {0x00, 0x00, 0x00, 0xFF}; // all dark
// uint8_t colorAmbient[4] = {0xFF, 0xFF, 0xFF, 0xFF}; // all light





///light stuff
typedef struct {
  color_t color;
  T3DVec3 dir;
} DirLight;


float rotAngle = 0.0f;
float lightCountTimer = 0.5f;

rspq_block_t *dplLight;

T3DMat4FP* lightMatFP;

DirLight dirLights[4] = {
  {.color = {0xFF, 0x00, 0x00, 0xFF}, .dir = {{ 1.0f,  1.0f, 1.0f}}},
  {.color = {0x00, 0xFF, 0x00, 0xFF}, .dir = {{-0.0f,  0.0f, 0.0f}}},
  {.color = {0x00, 0x00, 0xFF, 0xFF}, .dir = {{ 0.0f, -0.0f, 0.0f}}},
  {.color = {0x50, 0x50, 0x50, 0xFF}, .dir = {{ 0.0f,  0.0f, 0.0f}}}
};



///POINTLIGHT
typedef struct {
  T3DVec3 pos;
  float strength;
  color_t color;
} PointLight;


PointLight pointLight = {{{0.0f, 0.0f, 0.0f}}, 0.300f, {0x1F, 0x1F, 0x1F, 0xFF}};
T3DMat4FP* matLightFP;
rspq_block_t *dplLight;


//beamlight
// Define a new PointLight for the beam
PointLight beamPointLight = {{{0.0f, 0.0f, 0.0f}}, 0.100f, {0xFF, 0xFF, 0xFF, 0xFF}};
T3DMat4FP* beamMatLightFP;
rspq_block_t *beamDplLight;



// Function to load the beam as a point light
void load_beam_point_light() {
  beamMatLightFP = malloc_uncached(sizeof(T3DMat4FP)); // Allocate matrix for the beam light
  // T3DModel *modelBeamLight = t3d_model_load("rom://beam5.t3dm"); // Load the beam light model

  // Pre-render the beam light model into a display list
  rspq_block_begin();
  // t3d_model_draw(modelBeamLight);
  beamDplLight = rspq_block_end();
}


void draw_beam_point_light() {
  // Update the beam light's transformation matrix
  t3d_mat4fp_from_srt_euler(
    beamMatLightFP,
      (float[3]){0.001f, 0.001f, 0.001f}, // Scale
      // (float[3]){shotSize, shotSize, shotSize}, // Scale
      (float[3]){0.0f, 0.0f, 0.0f},    // Rotation
      (float[3]){                     // Translation
          beamPointLight.pos.v[0],
          beamPointLight.pos.v[1], // Slightly above the player
          beamPointLight.pos.v[2]
      }
  );

  // Set up the beam as a point light
  // t3d_light_set_ambient((uint8_t[]){10, 10, 10, 0xFF}); // Ambient light

  // if shotheld, fluctuate the beamlights strength
  if(shotHeld) {
    if (beamPointLight.strength >= 0.100f) {
      beamUp = false; // Start decreasing strength
    } else if (beamPointLight.strength <= 0.0250f) {
      beamUp = true; // Start increasing strength
    }
    if (beamUp) {
      shotSize += 0.004f; // Increase size
      beamPointLight.strength += 0.01f; // Increase strength
    } else {
      shotSize -= 0.004f; // Decrease size
      beamPointLight.strength -= 0.01f; // Decrease strength
    }
  }
  if (shotFired) {
    beamPointLight.strength = 0.200f; // Reset strength when shot is fired
  }


  t3d_light_set_point(1, &beamPointLight.color.r, &beamPointLight.pos, beamPointLight.strength, false);
  // t3d_light_set_count(2); // Include the beam light in the light count

  // Render the beam light
  t3d_matrix_push_pos(1);
  t3d_matrix_set(beamMatLightFP, true);
  rdpq_set_prim_color(beamPointLight.color);
  rspq_block_run(beamDplLight);
  t3d_matrix_pop(1);
}

// Function to update the beam's position when shotFired is true
void update_beam_point_light_position() {
  beamPointLight.pos.v[0] = cubePos.v[0];
  beamPointLight.pos.v[1] = cubePos.v[1] + 4.0f; // Slightly above the player
  beamPointLight.pos.v[2] = cubePos.v[2];
}


// Function to load the point light
void load_point_light() {
  matLightFP = malloc_uncached(sizeof(T3DMat4FP)); // Allocate matrix for the light
  T3DModel *modelLight = t3d_model_load("rom://light.t3dm"); // Load the light model

  // Pre-render the light model into a display list
  rspq_block_begin();
  t3d_model_draw(modelLight);
  dplLight = rspq_block_end();
}

void draw_point_light() {
  // Update the light's transformation matrix
  t3d_mat4fp_from_srt_euler(
      matLightFP,
      (float[3]){0.02f, 0.02f, 0.02f}, // Scale
      (float[3]){0.0f, 0.0f, 0.0f},    // Rotation
      (float[3]){                     // Translation
          pointLight.pos.v[0],
          pointLight.pos.v[1],
          pointLight.pos.v[2]
      }
  );

  // Set up the point light
  t3d_light_set_ambient((uint8_t[]){20, 20, 20, 0xFF}); // Ambient light
  t3d_light_set_point(0, &pointLight.color.r, &pointLight.pos, pointLight.strength, false);
  t3d_light_set_count(1);

  // Render the light
  t3d_matrix_push_pos(1);
  t3d_matrix_set(matLightFP, true);
  rdpq_set_prim_color(pointLight.color);
  rspq_block_run(dplLight);
  t3d_matrix_pop(1);
}



void load_directional_light() {
  lightMatFP = malloc_uncached(sizeof(T3DMat4FP) * 4);

  T3DModel *modelLight = t3d_model_load("rom:/light.t3dm");

  t3d_vec3_norm(&lightDirVec); // lightDirVec is a global variable from shared.h
  rspq_block_begin();
  t3d_model_draw(modelLight);
  dplLight = rspq_block_end();
}


void draw_directional_light() {
  rotAngle += 0.02f;
  lightCountTimer += 0.003f;

  for(int i = 0; i < 4; i++) {
    // rotate light around
    float lightRotSpeed = ((1.0f+i)*0.42f);
    dirLights[i].dir.v[0] = cosf(rotAngle * lightRotSpeed + i * 1.6f);
    dirLights[i].dir.v[1] = sinf(rotAngle * lightRotSpeed + i * 1.6f);
    dirLights[i].dir.v[2] = 0.0f;

    if(i % 2 == 0) {
      dirLights[i].dir.v[2] = dirLights[i].dir.v[1];
      dirLights[i].dir.v[1] = 0.0f;
    }

    t3d_mat4fp_from_srt_euler(&lightMatFP[i],
      (float[3]){0.02f, 0.02f, 0.02f},
      dirLights[i].dir.v,
      (float[3]){
        dirLights[i].dir.v[0] * 20.0f + i,
        dirLights[i].dir.v[1] + 20.0f,
        dirLights[i].dir.v[2] * 20.0f + i
      }
    );
  }

  //lighting
  int lightCount = fm_sinf(lightCountTimer) * 5.0f;
  lightCount = abs(lightCount);
  if(lightCount > 1)lightCount = 1;

  t3d_screen_clear_depth();
  t3d_light_set_ambient(lightColorAmbient);
  t3d_light_set_directional(1, colorDir, &lightDirVec);
  t3d_light_set_count(1);


  // lighting
  t3d_light_set_count(lightCount);
  // t3d_light_set_ambient(lightColorAmbient);
  t3d_matrix_push_pos(1);
  for(int i = 0; i < lightCount; i++) {
    t3d_light_set_directional(i, &dirLights[i].color.r, &dirLights[i].dir);
    t3d_matrix_set(&lightMatFP[i], true);
    rdpq_set_prim_color(dirLights[i].color);
    rspq_block_run(dplLight);
  }
  t3d_matrix_pop(1);
}

void add_ambient_light() {
  t3d_light_set_ambient(colorAmbient);
  t3d_vec3_norm(&lightDirVec);
  t3d_light_set_directional(0, colorDir, &lightDirVec);
  t3d_light_set_count(2); // Set the count to 2 to include the point light
}

void remove_beam_point_light() {
  rspq_block_free(beamDplLight);
}
