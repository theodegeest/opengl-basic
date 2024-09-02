#ifndef QUAD_H_
#define QUAD_H_

#include "vertex.h"

typedef struct {
  Vertex ld, lu, ru, rd;
} Quad;

Quad quad_create(float x, float y, float z, float width, float height, float depth, Color color, float texture_id);
void quad_print(Quad quad);

#endif // !QUAD_H_
