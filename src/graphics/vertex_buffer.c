#include "vertex_buffer.h"
#include <stdio.h>
#include <stdlib.h>

#include "../../include/glad/glad.h"
#include "debug.h"

static unsigned int _check_boundaries(VertexBuffer *vb);

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

void vertex_buffer_clear(VertexBuffer *vertexBuffer) { vertexBuffer->size = 0; }

void vertex_buffer_flush(VertexBuffer *vertexBuffer) {
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->id));
  GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, vertexBuffer->size * sizeof(Quad),
                         vertexBuffer->buffer));
}

void vertex_buffer_push_quad(VertexBuffer *vertexBuffer, Quad quad) {
  if (_check_boundaries(vertexBuffer)) {
    vertexBuffer->buffer[vertexBuffer->size++] = quad;
  } else {
    printf("[VertexBuffer Error] Buffer is full, could not push new Quad.\n");
  }
}

Quad *vertex_buffer_quad_get(VertexBuffer *vertexBuffer, unsigned int index) {
  if (index < vertexBuffer->size) {
    return &vertexBuffer->buffer[index];
  }

  return NULL;
}

void vertex_buffer_bind(VertexBuffer *vertexBuffer) {
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->id));
}
void vertex_buffer_unbind() { GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0)); }

/******************************************************************************
                              PRIVATE FUNCTIONS                               *
*******************************************************************************/

// Returns 1 if new push is allowed, else it returns 0.
static unsigned int _check_boundaries(VertexBuffer *vb) {
  if (vb->size == BUFFER_MAX_CAPACITY) {
    return 0;
  } else {
    return 1;
  }
}
