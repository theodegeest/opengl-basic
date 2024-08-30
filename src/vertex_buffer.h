#ifndef VERTEX_BUFFER_H_
#define VERTEX_BUFFER_H_

typedef struct {
  unsigned int id;
} VertexBuffer;

VertexBuffer *vertex_buffer_create(const void *data, unsigned int size);
void vertex_buffer_free(VertexBuffer *vertexBuffer);

void vertex_buffer_bind(VertexBuffer *vertexBuffer);
void vertex_buffer_unbind();

#endif // !VERTEX_BUFFER_H_
