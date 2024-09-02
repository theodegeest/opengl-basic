#include "renderer.h"
#include <stdlib.h>

#include "../../include/glad/glad.h"
#include "debug.h"

Renderer *renderer_create() {
  Renderer *renderer = malloc(sizeof(Renderer));

  return renderer;
}

void renderer_free(Renderer *renderer) { free(renderer); }

void renderer_clear(Renderer *renderer) {
  GLCall(glClear(GL_COLOR_BUFFER_BIT));
}

void renderer_set_clear_color(Renderer *renderer, float *rgba_values) {
  GLCall(glClearColor(rgba_values[0], rgba_values[1], rgba_values[2],
                      rgba_values[3]));
}

void renderer_draw(Renderer *renderer, VertexArray *va, VertexBuffer *vb,
                   Shader *shader) {
  shader_bind(shader);
  vertex_array_bind(va);
  // index_buffer_bind(ib);
  GLCall(glDrawElements(GL_TRIANGLES, 6 * vb->size, GL_UNSIGNED_INT, NULL));
}
