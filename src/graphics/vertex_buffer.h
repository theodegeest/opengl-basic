#ifndef VERTEX_BUFFER_H_
#define VERTEX_BUFFER_H_

#include "../mesh/quad.h"
#include <stdlib.h>

// #define BUFFER_MAX_CAPACITY 50000

typedef struct {
  unsigned int id;
  Vertex *buffer;
  unsigned int size;
  size_t max_capacity;
} VertexBuffer;

VertexBuffer *vertex_buffer_create(int number_of_vertices);
void vertex_buffer_free(VertexBuffer *vertexBuffer);

void vertex_buffer_clear(VertexBuffer *vertexBuffer);
void vertex_buffer_flush(VertexBuffer *vertexBuffer);
void vertex_buffer_flush_part(VertexBuffer *vertexBuffer, unsigned int index,
                              unsigned int number_of_vertices);

Vertex *vertex_buffer_push(VertexBuffer *vb, Vertex *vertices,
                           unsigned int number_of_vertices);
Vertex *vertex_buffer_get(VertexBuffer *vb, unsigned int index,
                          unsigned int number_of_vertices);
Quad *vertex_buffer_push_quad(VertexBuffer *vertexBuffer, Quad *quad);
Quad *vertex_buffer_quad_get(VertexBuffer *vertexBuffer, unsigned int index);

void vertex_buffer_bind(VertexBuffer *vertexBuffer);
void vertex_buffer_unbind();

#endif // !VERTEX_BUFFER_H_
