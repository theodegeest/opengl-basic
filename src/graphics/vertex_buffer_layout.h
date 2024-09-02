#ifndef VERTEX_BUFFER_LAYOUT_H_
#define VERTEX_BUFFER_LAYOUT_H_

#define VERTEX_BUFFER_LAYOUT_MAX_SIZE 5

typedef struct {
  unsigned int count;
  unsigned int type;
} VertexBufferElement;

unsigned int vertex_buffer_element_sizeof_type(unsigned int type);

typedef struct {
  unsigned int id;
  VertexBufferElement elements[VERTEX_BUFFER_LAYOUT_MAX_SIZE];
  unsigned int size;
  unsigned int stride;
} VertexBufferLayout;

VertexBufferLayout *vertex_buffer_layout_create();
void vertex_buffer_layout_free(VertexBufferLayout *layout);

void vertex_buffer_layout_push_float(VertexBufferLayout *layout,
                                     unsigned int count);

void vertex_buffer_layout_push_unsigned_int(VertexBufferLayout *layout,
                                            unsigned int count);

void vertex_buffer_layout_push_unsigned_char(VertexBufferLayout *layout,
                                             unsigned int count);

#endif // !VERTEX_BUFFER_LAYOUT_H_
