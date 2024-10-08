#include "index_buffer.h"
#include <stdio.h>
#include <stdlib.h>

#include "../../include/glad/glad.h"
#include "debug.h"

/******************************************************************************
                              PUBLIC FUNCTIONS                                *
*******************************************************************************/

IndexBuffer *index_buffer_create(const void *data, unsigned int count) {
  IndexBuffer *ib = malloc(sizeof(IndexBuffer));

  GLCall(glGenBuffers(1, &ib->id));
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->id));
  GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int),
                      data, GL_STATIC_DRAW));

  ib->count = count;

  return ib;
}

IndexBuffer *index_buffer_create_quad(int number_of_quads) {
  IndexBuffer *ib = malloc(sizeof(IndexBuffer));

  // Generate indices
  //

  // unsigned int indices[number_of_quads * 6];
  unsigned int *indices = malloc(number_of_quads * 6 * sizeof(unsigned int));

  unsigned int index_pattern[6] = {0, 1, 2, 2, 3, 0};

  for (int i = 0; i < number_of_quads; i++) {
    int start_index = i * 6;
    for (int j = 0; j < 6; j++) {
      int current_index = start_index + j;
      indices[current_index] = index_pattern[j] + (i * 4);
    }
  }

  GLCall(glGenBuffers(1, &ib->id));
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->id));
  GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                      number_of_quads * sizeof(unsigned int) * 6, indices,
                      GL_STATIC_DRAW));

  ib->count = number_of_quads;

  free(indices);

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
