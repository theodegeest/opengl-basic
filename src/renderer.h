#ifndef RENDERER_H_
#define RENDERER_H_

#include "index_buffer.h"
#include "shader.h"
#include "vertex_array.h"
typedef struct {

} Renderer;

Renderer *renderer_create();
void renderer_free(Renderer *renderer);

void renderer_clear(Renderer *renderer);
void renderer_set_clear_color(Renderer *renderer, float *rgba_values);
void renderer_draw(Renderer *renderer, VertexArray *va, IndexBuffer *ib,
                   Shader *shader);

#endif // !RENDERER_H_
