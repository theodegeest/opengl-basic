#include "../../include/glad/glad.h"
#include "scene_cube.h"
#include "../graphics/graphics.h"
#include "../graphics/index_buffer.h"
#include "../graphics/renderer.h"
#include "../graphics/shader.h"
#include "../graphics/texture.h"
#include "../graphics/vertex_array.h"
#include <cglm/cam.h>
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
  float value_depth;
  mat4 proj;
  mat4 view;
} CubeObj;

static void on_update(void *obj, float delta_time, GLFWwindow *window) {
  CubeObj *c_obj = (CubeObj *)obj;
  // printf("Clear Color On Update\n");

  // Create Quads
  vertex_buffer_clear(c_obj->vb);

  Quad left_quad =
      quad_create(200.0f, 200.0f, 0.0f, 100.0f, 100.0f, c_obj->value_depth,
                  (Color){0.0f, 0.0f, 0.0f, 1.0f}, 0);
  Quad right_quad =
      quad_create(350.0f, 200.0f, 0.0f, 100.0f, 100.0f, c_obj->value_depth * 2,
                  (Color){0.0f, 0.0f, 0.0f, 1.0f}, 0);
  // quad_print(left_quad);

  vertex_buffer_push_quad(c_obj->vb, &left_quad);
  vertex_buffer_push_quad(c_obj->vb, &right_quad);

  // printf("%f\n", c_obj->vb->buffer[4].ld.pos.x);
  // printf("%f\n", c_obj->vb->buffer[100].ld.pos.x);

  vertex_buffer_flush(c_obj->vb);
}

static void on_render(void *obj) {
  // printf("Clear Color On Render\n");
  CubeObj *c_obj = (CubeObj *)obj;
  renderer_clear(c_obj->renderer);

  // Use our shader program
  shader_bind(c_obj->shader);

  texture_bind(c_obj->texture, 0);

  {
    mat4 model;
    glm_translate_make(model, (vec3){0.0f, 0.0f, 0.0f});
    glm_translate(model,
                  (vec3){c_obj->value_x, c_obj->value_y, c_obj->value_z});

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
  CubeObj *c_obj = (CubeObj *)obj;

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
    nk_slider_float(context, -1.0f, &c_obj->value_z, 1.0f, 1.0f);
  }
  nk_layout_row_end(context);

  /* custom widget pixel width */
  nk_layout_row_begin(context, NK_STATIC, 25, 3);
  {
    nk_layout_row_push(context, 20);
    nk_label(context, "Depth:", NK_TEXT_LEFT);
    nk_layout_row_push(context, 30);
    char val[10];
    gcvt(c_obj->value_depth, 9, val);
    nk_label(context, val, NK_TEXT_LEFT);
    nk_layout_row_push(context, 110);
    nk_slider_float(context, -100.0f, &c_obj->value_depth, 100.0f, 1.0f);
  }
  nk_layout_row_end(context);
}

static void on_free(void *scene) {
  printf("Clear Color On Free\n");
  Scene *scene_p = (Scene *)scene;
  CubeObj *obj = (CubeObj *)scene_p->obj;

  vertex_array_free(obj->va);
  vertex_buffer_free(obj->vb);
  index_buffer_free(obj->ib);
  vertex_buffer_layout_free(obj->layout);
  shader_free(obj->shader);
  texture_free(obj->texture);
  renderer_free(obj->renderer);
  free(obj);
  free(scene);
}

Scene *scene_cube_init() {
  Scene *scene = malloc(sizeof(Scene));

  scene->on_update = &on_update;
  scene->on_render = &on_render;
  scene->on_ui_render = &on_ui_render;
  scene->on_free = &on_free;

  CubeObj *obj = malloc(sizeof(CubeObj));
  scene->obj = obj;

  obj->value_x = 0.0f;
  obj->value_y = 0.0f;
  obj->value_z = 0.0f;
  obj->value_depth = 0.0f;

  obj->renderer = renderer_create();

  renderer_set_clear_color(obj->renderer, (float[]){0.2f, 0.3f, 0.3f, 1.0f});

  obj->shader = shader_create("resources/shaders/cube.glsl");
  shader_bind(obj->shader);

  // Create and bind a Vertex Array Object
  obj->va = vertex_array_create();

  // Create and bind a Vertex Buffer Object
  obj->vb = vertex_buffer_create(4 * 2);

  obj->layout = vertex_buffer_layout_create();
  vertex_buffer_layout_push_float(obj->layout, 3);
  vertex_buffer_layout_push_float(obj->layout, 4);
  vertex_buffer_layout_push_float(obj->layout, 2);
  vertex_buffer_layout_push_float(obj->layout, 1);
  vertex_array_add_buffer(obj->va, obj->vb, obj->layout);

  // Create and bind a Index Buffer Object
  // obj->ib = index_buffer_create(indices, 6 * 3);
  obj->ib = index_buffer_create_quad(2);

  glm_ortho(0.0f, WINDOW_WIDTH, 0.0f, WINDOW_HEIGHT, -100.0f, 100.0f, obj->proj);

  glm_mat4_identity(obj->view);
  glm_translate(obj->view, (vec3){0.0f, 0.0f, 0.0f});

  obj->texture = texture_create("resources/textures/opengl.png");
  texture_bind(obj->texture, 0);

  int samplers[2] = {0, 1};
  shader_uniform_set_1iv(obj->shader, "u_Texture", 2, samplers);

  // Unbind the VBO and VAO
  vertex_array_unbind();
  vertex_buffer_unbind();
  index_buffer_unbind();

  printf("Clear Color Init\n");

  return scene;
}
