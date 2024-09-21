#ifndef QUAD_H_
#define QUAD_H_

#include "vertex.h"

typedef struct {
  Vertex ld, lu, ru, rd;
} Quad;

#define QUAD_NUMBER_OF_VERTICES (sizeof(Quad) / sizeof(Vertex))

Quad quad_create(float x, float y, float z, float width, float height,
                 float depth, Color color, float texture_id);
void quad_print(Quad quad);

float quad_texture_id_get(Quad *q);
void quad_texture_id_set(Quad *q, float texture_id);

void quad_color_set(Quad *q, Color color);
void quad_move(Quad *q, float x, float y, float z, float width, float height,
               float depth);

#endif // !QUAD_H_
