#include "index_buffer.h"
#include <stdlib.h>

#include "../include/glad/glad.h"
#include "debug.h"

/******************************************************************************
                              PUBLIC FUNCTIONS                                *
*******************************************************************************/

IndexBuffer *index_buffer_create(const void *data, unsigned int count) {
  IndexBuffer *ib = malloc(sizeof(IndexBuffer));

  GLCall(glGenBuffers(1, &ib->id));
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->id));
  GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data,
               GL_STATIC_DRAW));

  ib->count = count;

  return ib;
}

void index_buffer_free(IndexBuffer *indexBuffer) {
  GLCall(glDeleteBuffers(1, &indexBuffer->id));
  free(indexBuffer);
}

void index_buffer_bind(IndexBuffer *indexBuffer) {
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->id));
}
void index_buffer_unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

/******************************************************************************
                              PRIVATE FUNCTIONS                               *
*******************************************************************************/
