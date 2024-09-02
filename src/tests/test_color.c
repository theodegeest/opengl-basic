#include "test_color.h"
#include "../../include/glad/glad.h"
#include "../debug.h"
#include "../index_buffer.h"
#include "../renderer.h"
#include "../shader.h"
#include "../vertex_array.h"
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
  Renderer *renderer;
  struct nk_colorf color;
  mat4 proj;
  mat4 view;
} ColorObj;

static void on_update(void *obj, float delta_time) {
  ColorObj *c_obj = (ColorObj *)obj;
  // printf("Clear Color On Update\n");

  // Create Quads
  vertex_buffer_clear(c_obj->vb);

  Quad left_quad = quad_create(350.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0.0f,
                               (Color){c_obj->color.r, c_obj->color.g, c_obj->color.b, c_obj->color.a}, 0);

  vertex_buffer_push_quad(c_obj->vb, left_quad);

  // printf("%f\n", c_obj->vb->buffer[4].ld.pos.x);
  // printf("%f\n", c_obj->vb->buffer[100].ld.pos.x);

  vertex_buffer_flush(c_obj->vb);
}

static void on_render(void *obj) {
  // printf("Clear Color On Render\n");
  ColorObj *c_obj = (ColorObj *)obj;
  renderer_clear(c_obj->renderer);

  // Use our shader program
  shader_bind(c_obj->shader);

  {
    mat4 model;
    glm_translate_make(model, (vec3){0.0f, 0.0f, 0.0f});

    mat4 mvp;
    glm_mat4_mul(c_obj->proj, c_obj->view, mvp);
    glm_mat4_mul(mvp, model, mvp);

    shader_uniform_set_mat4f(c_obj->shader, "u_MVP", mvp);

    renderer_draw(c_obj->renderer, c_obj->va, c_obj->vb, c_obj->shader);
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
  ColorObj *c_obj = (ColorObj *)obj;

  nk_layout_row_begin(context, NK_STATIC, 20, 1);
  {
    nk_layout_row_push(context, 110);
    nk_label(context, "Color:", NK_TEXT_LEFT);
  }
  nk_layout_row_end(context);

  nk_layout_row_begin(context, NK_STATIC, 90, 1);
  {
    nk_layout_row_push(context, 150);
    c_obj->color = nk_color_picker(context, c_obj->color, NK_RGBA);
  }
  nk_layout_row_end(context);
}

static void on_free(void *test) {
  printf("Clear Color On Free\n");
  Test *test_p = (Test *)test;
  ColorObj *obj = (ColorObj *)test_p->obj;

  vertex_array_free(obj->va);
  vertex_buffer_free(obj->vb);
  index_buffer_free(obj->ib);
  vertex_buffer_layout_free(obj->layout);
  shader_free(obj->shader);
  renderer_free(obj->renderer);
  free(obj);
  free(test);
}

Test *test_color_init() {
  Test *test = malloc(sizeof(Test));

  test->on_update = &on_update;
  test->on_render = &on_render;
  test->on_ui_render = &on_ui_render;
  test->on_free = &on_free;

  ColorObj *obj = malloc(sizeof(ColorObj));
  test->obj = obj;

  obj->color = (struct nk_colorf){1.0f, 0.0f, 0.0f, 1.0f};

  obj->renderer = renderer_create();

  renderer_set_clear_color(obj->renderer, (float[]){0.2f, 0.3f, 0.3f, 1.0f});

  obj->shader = shader_create("resources/shaders/color.glsl");
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

  // Unbind the VBO and VAO
  vertex_array_unbind();
  vertex_buffer_unbind();
  index_buffer_unbind();

  printf("Clear Color Init\n");

  return test;
}
