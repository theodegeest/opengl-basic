#include "renderer.h"
#include <stdlib.h>

#include "../include/glad/glad.h"
#include "index_buffer.h"
#include "debug.h"

Renderer *renderer_create() {
  Renderer *renderer = malloc(sizeof(Renderer));

  return renderer;
}

void renderer_free(Renderer *renderer) { free(renderer); }

void renderer_clear(Renderer *renderer) { GLCall(glClear(GL_COLOR_BUFFER_BIT)); }

void renderer_draw(Renderer *renderer, VertexArray *va, IndexBuffer *ib,
                   Shader *shader) {
  shader_bind(shader);
  vertex_array_bind(va);
  index_buffer_bind(ib);
  GLCall(glDrawElements(GL_TRIANGLES, ib->count, GL_UNSIGNED_INT, NULL));
}
