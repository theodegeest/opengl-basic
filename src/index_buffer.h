#ifndef INDEX_BUFFER_H_
#define INDEX_BUFFER_H_

#define MAX_INDEX_BUFFER_CAPACITY 10000

typedef struct {
  unsigned int id;
  unsigned int count;
} IndexBuffer;

IndexBuffer *index_buffer_create(const void *data, unsigned int count);
IndexBuffer *index_buffer_create_quad();
void index_buffer_free(IndexBuffer *indexBuffer);

void index_buffer_bind(IndexBuffer *indexBuffer);
void index_buffer_unbind();

#endif // !INDEX_BUFFER_H_
