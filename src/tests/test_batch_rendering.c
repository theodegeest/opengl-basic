#include "test_batch_rendering.h"
#include "../../include/glad/glad.h"
#include "../graphics/index_buffer.h"
#include "../graphics/renderer.h"
#include "../graphics/shader.h"
#include "../graphics/texture.h"
#include "../graphics/vertex_array.h"
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../include/Nuklear/nuklear.h"

typedef struct {
  VertexArray *va;
  VertexBuffer *vb;
  IndexBuffer *ib;
  VertexBufferLayout *layout;
  Shader *shader;
  Texture *texture;
  Renderer *renderer;
  float value_x;
  float value_y;
  float value_z;
  mat4 proj;
  mat4 view;
} BatchRenderingObj;

static void on_update(void *obj, float delta_time) {
  BatchRenderingObj *b_obj = (BatchRenderingObj *)obj;
  // printf("Clear Color On Update\n");

  // Create Quads
  vertex_buffer_clear(b_obj->vb);

  int size = 10;

  for (int i = 0; i < 70; i++) {
    for (int j = 0; j < 50; j++) {
      Quad q = quad_create(50.0f + i * size, 50.0f + j * size, 0.0f, size, size,
                           0.0f, (Color){0.0f, 0.0f, 0.0f, 1.0f}, 0);
      vertex_buffer_push_quad(b_obj->vb, q);
    }
  }

  vertex_buffer_flush(b_obj->vb);
}

static void on_render(void *obj) {
  // printf("Clear Color On Render\n");
  BatchRenderingObj *b_obj = (BatchRenderingObj *)obj;
  renderer_clear(b_obj->renderer);

  // Use our shader program
  shader_bind(b_obj->shader);

  texture_bind(b_obj->texture, 0);

  {
    mat4 model;
    glm_translate_make(model, (vec3){0.0f, 0.0f, 0.0f});
    glm_translate(model,
                  (vec3){b_obj->value_x, b_obj->value_y, b_obj->value_z});

    mat4 mvp;
    glm_mat4_mul(b_obj->proj, b_obj->view, mvp);
    glm_mat4_mul(mvp, model, mvp);

    shader_uniform_set_mat4f(b_obj->shader, "u_MVP", mvp);

    renderer_draw(b_obj->renderer, b_obj->va, b_obj->vb, b_obj->shader);
  }

  // {
  //   mat4 model;
  //   glm_translate_make(model, (vec3){300.0f, 100.0f, 0.0f});
  //   glm_translate(
  //       model, (vec3){c_obj->value_x * 2, c_obj->value_y * 2,
  //       c_obj->value_z});
  //
  //   mat4 mvp;
  //   glm_mat4_mul(c_obj->proj, c_obj->view, mvp);
  //   glm_mat4_mul(mvp, model, mvp);
  //
  //   shader_uniform_set_mat4f(c_obj->shader, "u_MVP", mvp);
  //
  //   renderer_draw(c_obj->renderer, c_obj->va, c_obj->ib, c_obj->shader);
  // }

  vertex_array_unbind();
}

static void on_ui_render(void *obj, void *context) {
  BatchRenderingObj *b_obj = (BatchRenderingObj *)obj;

  /* custom widget pixel width */
  nk_layout_row_begin(context, NK_STATIC, 25, 3);
  {
    nk_layout_row_push(context, 20);
    nk_label(context, "X:", NK_TEXT_LEFT);
    nk_layout_row_push(context, 30);
    char val[10];
    gcvt(b_obj->value_x, 9, val);
    nk_label(context, val, NK_TEXT_LEFT);
    nk_layout_row_push(context, 110);
    nk_slider_float(context, -100.0f, &b_obj->value_x, 100.0f, 1.0f);
  }
  nk_layout_row_end(context);

  /* custom widget pixel width */
  nk_layout_row_begin(context, NK_STATIC, 25, 3);
  {
    nk_layout_row_push(context, 20);
    nk_label(context, "Y:", NK_TEXT_LEFT);
    nk_layout_row_push(context, 30);
    char val[10];
    gcvt(b_obj->value_y, 9, val);
    nk_label(context, val, NK_TEXT_LEFT);
    nk_layout_row_push(context, 110);
    nk_slider_float(context, -100.0f, &b_obj->value_y, 100.0f, 1.0f);
  }
  nk_layout_row_end(context);

  /* custom widget pixel width */
  nk_layout_row_begin(context, NK_STATIC, 25, 3);
  {
    nk_layout_row_push(context, 20);
    nk_label(context, "Z:", NK_TEXT_LEFT);
    nk_layout_row_push(context, 30);
    char val[10];
    gcvt(b_obj->value_z, 9, val);
    nk_label(context, val, NK_TEXT_LEFT);
    nk_layout_row_push(context, 110);
    nk_slider_float(context, -1.0f, &b_obj->value_z, 1.0f, 1.0f);
  }
  nk_layout_row_end(context);
}

static void on_free(void *test) {
  printf("Clear Color On Free\n");
  Test *test_p = (Test *)test;
  BatchRenderingObj *obj = (BatchRenderingObj *)test_p->obj;

  vertex_array_free(obj->va);
  vertex_buffer_free(obj->vb);
  index_buffer_free(obj->ib);
  vertex_buffer_layout_free(obj->layout);
  shader_free(obj->shader);
  texture_free(obj->texture);
  renderer_free(obj->renderer);
  free(obj);
  free(test);
}

Test *test_batch_rendering_init() {
  Test *test = malloc(sizeof(Test));

  test->on_update = &on_update;
  test->on_render = &on_render;
  test->on_ui_render = &on_ui_render;
  test->on_free = &on_free;

  BatchRenderingObj *obj = malloc(sizeof(BatchRenderingObj));
  test->obj = obj;

  obj->value_x = 0.0f;
  obj->value_y = 0.0f;
  obj->value_z = 0.0f;

  obj->renderer = renderer_create();

  renderer_set_clear_color(obj->renderer, (float[]){0.2f, 0.3f, 0.3f, 1.0f});

  obj->shader = shader_create("resources/shaders/batch-rendering.glsl");
  shader_bind(obj->shader);

  // Create and bind a Vertex Array Object
  obj->va = vertex_array_create();

  // Create and bind a Vertex Buffer Object
  obj->vb = vertex_buffer_create();

  obj->layout = vertex_buffer_layout_create();
  vertex_buffer_layout_push_float(obj->layout, 3);
  vertex_buffer_layout_push_float(obj->layout, 4);
  vertex_buffer_layout_push_float(obj->layout, 2);
  vertex_buffer_layout_push_float(obj->layout, 1);
  vertex_array_add_buffer(obj->va, obj->vb, obj->layout);

  // Create and bind a Index Buffer Object
  // obj->ib = index_buffer_create(indices, 6 * 3);
  obj->ib = index_buffer_create_quad();

  glm_ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f, obj->proj);

  glm_mat4_identity(obj->view);
  glm_translate(obj->view, (vec3){0.0f, 0.0f, 0.0f});

  obj->texture = texture_create("resources/textures/small-circle.png");
  texture_bind(obj->texture, 0);

  int samplers[2] = {0, 1};
  shader_uniform_set_1iv(obj->shader, "u_Texture", 2, samplers);

  // Unbind the VBO and VAO
  vertex_array_unbind();
  vertex_buffer_unbind();
  index_buffer_unbind();

  printf("Clear Color Init\n");

  return test;
}
