#include "vertex_buffer.h"
#include <stdlib.h>

#include "../include/glad/glad.h"
#include "debug.h"

/******************************************************************************
                              PUBLIC FUNCTIONS                                *
*******************************************************************************/

VertexBuffer *vertex_buffer_create(const void *data, unsigned int size) {
  VertexBuffer *vb = malloc(sizeof(VertexBuffer));

  GLCall(glGenBuffers(1, &vb->id));
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, vb->id));
  GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));

  return vb;
}

void vertex_buffer_free(VertexBuffer *vertexBuffer) {
  GLCall(glDeleteBuffers(1, &vertexBuffer->id));
  free(vertexBuffer);
}

void vertex_buffer_bind(VertexBuffer *vertexBuffer) {
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->id));
}
void vertex_buffer_unbind() { GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0)); }

/******************************************************************************
                              PRIVATE FUNCTIONS                               *
*******************************************************************************/
