#include "vertex_buffer_layout.h"
#include <stdlib.h>

#include "../../include/glad/glad.h"
#include <stdio.h>

static void _check_layout_size(VertexBufferLayout *layout);

/******************************************************************************
                              PUBLIC FUNCTIONS                                *
*******************************************************************************/

unsigned int vertex_buffer_element_sizeof_type(unsigned int type) {
  switch (type) {
  case GL_FLOAT:
    return 4;
  case GL_UNSIGNED_INT:
    return 4;
  case GL_UNSIGNED_BYTE:
    return 1;
  }
  return 0;
}

VertexBufferLayout *vertex_buffer_layout_create() {
  VertexBufferLayout *layout = malloc(sizeof(VertexBufferLayout));

  layout->size = 0;
  layout->stride = 0;

  return layout;
}

void vertex_buffer_layout_free(VertexBufferLayout *layout) { free(layout); }

// void vertex_buffer__layout_bind(VertexBufferLayout *layout) {
//   // glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->id);
// }
// void vertex_buffer__layout_unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

void vertex_buffer_layout_push_float(VertexBufferLayout *layout,
                                     unsigned int count) {
  _check_layout_size(layout);

  VertexBufferElement element = {count, GL_FLOAT};
  layout->elements[layout->size++] = element;

  layout->stride += sizeof(GLfloat) * count;
}

void vertex_buffer_layout_push_unsigned_int(VertexBufferLayout *layout,
                                            unsigned int count) {
  _check_layout_size(layout);

  VertexBufferElement element = {count, GL_UNSIGNED_INT};
  layout->elements[layout->size++] = element;

  layout->stride += sizeof(GLuint) * count;
}

void vertex_buffer_layout_push_unsigned_char(VertexBufferLayout *layout,
                                             unsigned int count) {
  _check_layout_size(layout);

  VertexBufferElement element = {count, GL_UNSIGNED_BYTE};
  layout->elements[layout->size++] = element;

  layout->stride += sizeof(GLbyte) * count;
}

/******************************************************************************
                              PRIVATE FUNCTIONS                               *
*******************************************************************************/

static void _check_layout_size(VertexBufferLayout *layout) {
  if (layout->size == VERTEX_BUFFER_LAYOUT_MAX_SIZE) {
    printf("Vertex Buffer Layout has a max size of %d\n",
           VERTEX_BUFFER_LAYOUT_MAX_SIZE);
    exit(EXIT_FAILURE);
  }
}
