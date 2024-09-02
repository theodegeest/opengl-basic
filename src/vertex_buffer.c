#include "vertex_buffer.h"
#include <stdio.h>
#include <stdlib.h>

#include "../include/glad/glad.h"
#include "debug.h"

/******************************************************************************
                              PUBLIC FUNCTIONS                                *
*******************************************************************************/

VertexBuffer *vertex_buffer_create() {
  VertexBuffer *vb = malloc(sizeof(VertexBuffer));

  GLCall(glGenBuffers(1, &vb->id));
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, vb->id));
  GLCall(glBufferData(GL_ARRAY_BUFFER, BUFFER_MAX_CAPACITY * sizeof(Quad), NULL,
                      GL_DYNAMIC_DRAW));

  vb->buffer = malloc(BUFFER_MAX_CAPACITY * sizeof(Quad));
  vb->size = 0;

  return vb;
}

void vertex_buffer_free(VertexBuffer *vertexBuffer) {
  GLCall(glDeleteBuffers(1, &vertexBuffer->id));
  free(vertexBuffer->buffer);
  free(vertexBuffer);
}

Quad quad_create(int x, int y, int width, int height, Color color,
                 float texture_id) {
  Vertex ld;
  ld.pos = (Vec3){x, y, 0.0f};
  ld.color = color;
  ld.texture_coord = (Vec2){0.0f, 0.0f};
  ld.texture_id = texture_id;

  Vertex lu;
  lu.pos = (Vec3){x, y + height, 0.0f};
  lu.color = color;
  lu.texture_coord = (Vec2){0.0f, 1.0f};
  lu.texture_id = texture_id;

  Vertex ru;
  ru.pos = (Vec3){x + width, y + height, 0.0f};
  ru.color = color;
  ru.texture_coord = (Vec2){1.0f, 1.0f};
  ru.texture_id = texture_id;

  Vertex rd;
  rd.pos = (Vec3){x + width, y, 0.0f};
  rd.color = color;
  rd.texture_coord = (Vec2){1.0f, 0.0f};
  rd.texture_id = texture_id;

  return (Quad){ld, lu, ru, rd};
}

void quad_print(Quad quad) {
  printf("Quad(x: %f, y: %f, w: %f, h: %f)\n", quad.ld.pos.x, quad.ld.pos.y,
         quad.rd.pos.x - quad.ld.pos.x, quad.lu.pos.y - quad.ld.pos.y);
}

void vertex_buffer_clear(VertexBuffer *vertexBuffer) { vertexBuffer->size = 0; }

void vertex_buffer_flush(VertexBuffer *vertexBuffer) {
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->id));
  GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, vertexBuffer->size * sizeof(Quad),
                         vertexBuffer->buffer));
}

void vertex_buffer_push_quad(VertexBuffer *vertexBuffer, Quad quad) {
  vertexBuffer->buffer[vertexBuffer->size++] = quad;
}

void vertex_buffer_bind(VertexBuffer *vertexBuffer) {
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->id));
}
void vertex_buffer_unbind() { GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0)); }

/******************************************************************************
                              PRIVATE FUNCTIONS                               *
*******************************************************************************/
