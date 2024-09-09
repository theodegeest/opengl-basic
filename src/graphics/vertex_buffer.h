#ifndef VERTEX_BUFFER_H_
#define VERTEX_BUFFER_H_

#include "../mesh/quad.h"

#define BUFFER_MAX_CAPACITY 50000

typedef struct {
  unsigned int id;
  Vertex *buffer;
  unsigned int size;
} VertexBuffer;

VertexBuffer *vertex_buffer_create(int number_of_vertices);
void vertex_buffer_free(VertexBuffer *vertexBuffer);

void vertex_buffer_clear(VertexBuffer *vertexBuffer);
void vertex_buffer_flush(VertexBuffer *vertexBuffer);

void vertex_buffer_push_quad(VertexBuffer *vertexBuffer, Quad quad);
Quad *vertex_buffer_quad_get(VertexBuffer *vertexBuffer, unsigned int index);

void vertex_buffer_bind(VertexBuffer *vertexBuffer);
void vertex_buffer_unbind();

#endif // !VERTEX_BUFFER_H_
