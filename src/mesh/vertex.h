#ifndef VERTEX_H_
#define VERTEX_H_

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

// Vertex vertex_create();

#endif // !VERTEX_H_
