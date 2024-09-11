#ifndef SHAPE_H_
#define SHAPE_H_

#include "quad.h"

typedef struct {
  Quad left, up, right, down;
} ShapeBox;

#define SHAPE_BOX_NUMBER_OF_VERTICES sizeof(ShapeBox) / sizeof(Vertex)

ShapeBox shape_box_create(float x, float y, float width, float height,
                          float thickness, Color color, float texture_id);

#endif // !SHAPE_H_
