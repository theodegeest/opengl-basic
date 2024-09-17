#include "quad.h"

#include <stdio.h>

Quad quad_create(float x, float y, float z, float width, float height, float depth,
                 Color color, float texture_id) {
  Vertex ld;
  ld.pos = (Vec3){x, y, z};
  ld.color = color;
  ld.texture_coord = (Vec2){0.0f, 0.0f};
  ld.texture_id = texture_id;

  Vertex lu;
  lu.pos = (Vec3){x, y + height, z};
  lu.color = color;
  lu.texture_coord = (Vec2){0.0f, 1.0f};
  lu.texture_id = texture_id;

  Vertex ru;
  ru.pos = (Vec3){x + width, y + height, z + depth};
  ru.color = color;
  ru.texture_coord = (Vec2){1.0f, 1.0f};
  ru.texture_id = texture_id;

  Vertex rd;
  rd.pos = (Vec3){x + width, y, z + depth};
  rd.color = color;
  rd.texture_coord = (Vec2){1.0f, 0.0f};
  rd.texture_id = texture_id;

  return (Quad){ld, lu, ru, rd};
}

void quad_print(Quad quad) {
  printf("Quad(x: %f, y: %f, w: %f, h: %f)\n", quad.ld.pos.x, quad.ld.pos.y,
         quad.rd.pos.x - quad.ld.pos.x, quad.lu.pos.y - quad.ld.pos.y);
}

float quad_texture_id_get(Quad *q) {
  return q->ld.texture_id;
}

void quad_texture_id_set(Quad *q, float texture_id) {
  q->ld.texture_id = texture_id;
  q->lu.texture_id = texture_id;
  q->ru.texture_id = texture_id;
  q->rd.texture_id = texture_id;
}

void quad_color_set(Quad *q, Color color) {
  q->ld.color = color;
  q->lu.color = color;
  q->ru.color = color;
  q->rd.color = color;
}
