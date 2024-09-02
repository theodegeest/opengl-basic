#ifndef QUAD_H_
#define QUAD_H_

#include "vertex.h"

typedef struct {
  Vertex ld, lu, ru, rd;
} Quad;

Quad quad_create(int x, int y, int width, int height, Color color, float texture_id);
void quad_print(Quad quad);

#endif // !QUAD_H_
