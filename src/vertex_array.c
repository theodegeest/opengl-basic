#include "vertex_array.h"

#include "../include/glad/glad.h"
#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************
                              PUBLIC FUNCTIONS                                *
*******************************************************************************/

VertexArray *vertex_array_create() {
  VertexArray *va = malloc(sizeof(VertexArray));

  glGenVertexArrays(1, &va->id);

  return va;
}

void vertex_array_free(VertexArray *vertexArray) {
  glDeleteVertexArrays(1, &vertexArray->id);
  free(vertexArray);
}

void vertex_array_add_buffer(VertexArray *va, VertexBuffer *vb,
                             VertexBufferLayout *layout) {
  vertex_array_bind(va);
  vertex_buffer_bind(vb);

  VertexBufferElement *elements = layout->elements;
  unsigned int offset = 0;

  for (unsigned int i = 0; i < layout->size; i++) {
    VertexBufferElement element = elements[i];

    glEnableVertexAttribArray(i);
    glVertexAttribPointer(i, element.count, element.type, GL_FALSE,
                          layout->stride, (void *)(size_t)offset);

    offset += element.count * vertex_buffer_element_sizeof_type(element.type);
  }
}

void vertex_array_bind(VertexArray *va) { glBindVertexArray(va->id); }
void vertex_array_unbind() { glBindVertexArray(0); }

/******************************************************************************
                              PRIVATE FUNCTIONS                               *
*******************************************************************************/
