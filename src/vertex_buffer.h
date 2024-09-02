#ifndef VERTEX_BUFFER_H_
#define VERTEX_BUFFER_H_

#include "mesh/quad.h"

#define BUFFER_MAX_CAPACITY 1000

typedef struct {
  unsigned int id;
  Quad *buffer;
  unsigned int size;
} VertexBuffer;

VertexBuffer *vertex_buffer_create();
void vertex_buffer_free(VertexBuffer *vertexBuffer);

void vertex_buffer_clear(VertexBuffer *vertexBuffer);
void vertex_buffer_flush(VertexBuffer *vertexBuffer);

void vertex_buffer_push_quad(VertexBuffer *vertexBuffer, Quad quad);

void vertex_buffer_bind(VertexBuffer *vertexBuffer);
void vertex_buffer_unbind();

#endif // !VERTEX_BUFFER_H_
