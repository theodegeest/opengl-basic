#include "test_texture.h"
#include "../../include/glad/glad.h"
#include "../debug.h"
#include "../index_buffer.h"
#include "../renderer.h"
#include "../shader.h"
#include "../texture.h"
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
  Texture *texture;
  Renderer *renderer;
  float value_x;
  float value_y;
  float value_z;
  mat4 proj;
  mat4 view;
} ClearColorObj;

static void on_update(void *obj, float delta_time) {
  // printf("Clear Color On Update\n");
}

static void on_render(void *obj) {
  // printf("Clear Color On Render\n");
  ClearColorObj *c_obj = (ClearColorObj *)obj;
  // GLCall(glClearColor(c_obj->clear_color.r, c_obj->clear_color.g,
  //                     c_obj->clear_color.b, c_obj->clear_color.a));
  // GLCall(glClear(GL_COLOR_BUFFER_BIT));
  renderer_clear(c_obj->renderer);

  // Use our shader program
  shader_bind(c_obj->shader);

  texture_bind(c_obj->texture, 0);

  // shader_uniform_set_4f(shader, "u_Color", r, 0.3f, 0.8f, 1.0f);

  {
    mat4 model;
    // glm_mat4_identity(model);
    glm_translate_make(model,
                       (vec3){c_obj->value_x, c_obj->value_y, c_obj->value_z});

    mat4 mvp;
    glm_mat4_mul(c_obj->proj, c_obj->view, mvp);
    glm_mat4_mul(mvp, model, mvp);

    shader_uniform_set_mat4f(c_obj->shader, "u_MVP", mvp);

    renderer_draw(c_obj->renderer, c_obj->va, c_obj->ib, c_obj->shader);
  }

  {
    mat4 model;
    // glm_mat4_identity(model);
    glm_translate_make(model, (vec3){300.0f, 100.0f, 0.0f});
    glm_translate(
        model, (vec3){c_obj->value_x * 2, c_obj->value_y * 2, c_obj->value_z});

    mat4 mvp;
    glm_mat4_mul(c_obj->proj, c_obj->view, mvp);
    glm_mat4_mul(mvp, model, mvp);

    shader_uniform_set_mat4f(c_obj->shader, "u_MVP", mvp);

    renderer_draw(c_obj->renderer, c_obj->va, c_obj->ib, c_obj->shader);
  }
  // glClearError();
  // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
  // glCheckError();

  vertex_array_unbind();
}

static void on_ui_render(void *obj, void *context) {
  ClearColorObj *c_obj = (ClearColorObj *)obj;

  /* custom widget pixel width */
  nk_layout_row_begin(context, NK_STATIC, 25, 3);
  {
    nk_layout_row_push(context, 20);
    nk_label(context, "X:", NK_TEXT_LEFT);
    nk_layout_row_push(context, 30);
    char val[10];
    gcvt(c_obj->value_x, 9, val);
    nk_label(context, val, NK_TEXT_LEFT);
    nk_layout_row_push(context, 110);
    nk_slider_float(context, -100.0f, &c_obj->value_x, 100.0f, 1.0f);
  }
  nk_layout_row_end(context);

  /* custom widget pixel width */
  nk_layout_row_begin(context, NK_STATIC, 25, 3);
  {
    nk_layout_row_push(context, 20);
    nk_label(context, "Y:", NK_TEXT_LEFT);
    nk_layout_row_push(context, 30);
    char val[10];
    gcvt(c_obj->value_y, 9, val);
    nk_label(context, val, NK_TEXT_LEFT);
    nk_layout_row_push(context, 110);
    nk_slider_float(context, -100.0f, &c_obj->value_y, 100.0f, 1.0f);
  }
  nk_layout_row_end(context);

  /* custom widget pixel width */
  nk_layout_row_begin(context, NK_STATIC, 25, 3);
  {
    nk_layout_row_push(context, 20);
    nk_label(context, "Z:", NK_TEXT_LEFT);
    nk_layout_row_push(context, 30);
    char val[10];
    gcvt(c_obj->value_z, 9, val);
    nk_label(context, val, NK_TEXT_LEFT);
    nk_layout_row_push(context, 110);
    nk_slider_float(context, -100.0f, &c_obj->value_z, 100.0f, 1.0f);
  }
  nk_layout_row_end(context);

  // nk_layout_row_push(context, 110);
  // nk_label(context, "Clear Color:", NK_TEXT_LEFT);
  // nk_layout_row_begin(context, NK_STATIC, 90, 1);
  // {
  //   nk_layout_row_push(context, 150);
  //   c_obj->clear_color =
  //       nk_color_picker(context, c_obj->clear_color, NK_RGBA);
  // }
}

static void on_free(void *test) {
  printf("Clear Color On Free\n");
  Test *test_p = (Test *)test;
  ClearColorObj *obj = (ClearColorObj *)test_p->obj;

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

Test *test_texture_init() {
  Test *test = malloc(sizeof(Test));

  test->on_update = &on_update;
  test->on_render = &on_render;
  test->on_ui_render = &on_ui_render;
  test->on_free = &on_free;

  ClearColorObj *obj = malloc(sizeof(ClearColorObj));
  test->obj = obj;

  // obj->clear_color = (struct nk_colorf){0.2f, 0.3f, 0.3f, 1.0f};

  obj->value_x = 0.0f;
  obj->value_y = 0.0f;
  obj->value_z = 0.0f;

  obj->renderer = renderer_create();

  renderer_set_clear_color(obj->renderer, (float[]){0.2f, 0.3f, 0.3f, 1.0f});

  obj->shader = shader_create("resources/shaders/basic.glsl");
  shader_bind(obj->shader);

  float vertices[] = {-50.0f, -50.0f, 0.0f,  0.0f,  0.0f,  50.0f, -50.0f,
                      0.0f,   1.0f,   0.0f,  50.0f, 50.0f, 0.0f,  1.0f,
                      1.0f,   -50.0f, 50.0f, 0.0f,  0.0f,  1.0f};

  unsigned int indices[] = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  // Create and bind a Vertex Array Object
  obj->va = vertex_array_create();

  // Create and bind a Vertex Buffer Object
  obj->vb = vertex_buffer_create(vertices, sizeof(vertices));

  obj->layout = vertex_buffer_layout_create();
  vertex_buffer_layout_push_float(obj->layout, 3);
  vertex_buffer_layout_push_float(obj->layout, 2);
  vertex_array_add_buffer(obj->va, obj->vb, obj->layout);
  // glCheckError();

  // Create and bind a Index Buffer Object
  obj->ib = index_buffer_create(indices, 6);

  glm_ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f, obj->proj);

  glm_mat4_identity(obj->view);
  glm_translate(obj->view, (vec3){0.0f, 0.0f, 0.0f});

  // glClearError();
  obj->texture = texture_create("resources/textures/opengl.png");
  // glCheckError();
  texture_bind(obj->texture, 0);
  // glClearError();
  shader_uniform_set_1i(obj->shader, "u_Texture", 0);
  // glCheckError();

  // Unbind the VBO and VAO
  vertex_array_unbind();
  vertex_buffer_unbind();
  index_buffer_unbind();

  printf("Clear Color Init\n");

  return test;
}
