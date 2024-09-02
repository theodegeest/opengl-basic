#ifndef VERTEX_ARRAY_H_
#define VERTEX_ARRAY_H_

#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"

typedef struct {
  unsigned int id;
} VertexArray;

VertexArray *vertex_array_create();
void vertex_array_free(VertexArray *vertexArray);

void vertex_array_add_buffer(VertexArray *va, VertexBuffer *vb, VertexBufferLayout *layout);

void vertex_array_bind(VertexArray *va);
void vertex_array_unbind();

#endif // !VERTEX_ARRAY_H_
