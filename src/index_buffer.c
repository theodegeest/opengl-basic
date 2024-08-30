#include "index_buffer.h"
#include <stdlib.h>

#include "../include/glad/glad.h"

/******************************************************************************
                              PUBLIC FUNCTIONS                                *
*******************************************************************************/

IndexBuffer *index_buffer_create(const void *data, unsigned int count) {
  IndexBuffer *ib = malloc(sizeof(IndexBuffer));

  glGenBuffers(1, &ib->id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data,
               GL_STATIC_DRAW);

  ib->count = count;

  return ib;
}

void index_buffer_free(IndexBuffer *indexBuffer) {
  glDeleteBuffers(1, &indexBuffer->id);
  free(indexBuffer);
}

void index_buffer_bind(IndexBuffer *indexBuffer) {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->id);
}
void index_buffer_unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

/******************************************************************************
                              PRIVATE FUNCTIONS                               *
*******************************************************************************/
