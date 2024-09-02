#ifndef VERTEX_BUFFER_H_
#define VERTEX_BUFFER_H_


typedef struct {
  float x, y;
} Vec2;

typedef struct {
  float x, y, z;
} Vec3;

typedef struct {
  float r, g, b, a;
} Color;

typedef struct {
  Vec3 pos;
  Color color;
  Vec2 texture_coord;
  float texture_id;
} Vertex;

typedef struct {
  Vertex ld, lu, ru, rd;
} Quad;

#define BUFFER_MAX_CAPACITY 3

typedef struct {
  unsigned int id;
  Quad *buffer;
  unsigned int size;
} VertexBuffer;

VertexBuffer *vertex_buffer_create();
void vertex_buffer_free(VertexBuffer *vertexBuffer);

void vertex_buffer_clear(VertexBuffer *vertexBuffer);
void vertex_buffer_flush(VertexBuffer *vertexBuffer);

void vertex_buffer_push_quad(VertexBuffer *vertexBuffer, Quad quad);

Quad quad_create(int x, int y, int width, int height, Color color, float texture_id);
void quad_print(Quad quad);

void vertex_buffer_bind(VertexBuffer *vertexBuffer);
void vertex_buffer_unbind();

#endif // !VERTEX_BUFFER_H_
