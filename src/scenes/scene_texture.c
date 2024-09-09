#include "scene_texture.h"
#include "../../include/glad/glad.h"
#include "../graphics/index_buffer.h"
#include "../graphics/renderer.h"
#include "../graphics/shader.h"
#include "../graphics/texture.h"
#include "../graphics/vertex_array.h"
#include "../graphics/graphics.h"
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
} TextureObj;

static void on_update(void *obj, float delta_time) {
  TextureObj *t_obj = (TextureObj *)obj;
  // printf("Clear Color On Update\n");

  // Create Quads
  vertex_buffer_clear(t_obj->vb);

  Quad left_quad = quad_create(100.0f, 200.0f, 0.0f, 100.0f, 100.0f, 0.0f,
                               (Color){0.0f, 0.0f, 0.0f, 1.0f}, 0);
  Quad right_quad = quad_create(250.0f, 200.0f, 0.0f, 100.0f, 100.0f, 0.0f,
                                (Color){0.0f, 0.0f, 0.0f, 1.0f}, 0);
  // quad_print(left_quad);

  vertex_buffer_push_quad(t_obj->vb, left_quad);
  vertex_buffer_push_quad(t_obj->vb, right_quad);

  vertex_buffer_flush(t_obj->vb);
}

static void on_render(void *obj) {
  // printf("Clear Color On Render\n");
  TextureObj *t_obj = (TextureObj *)obj;
  renderer_clear(t_obj->renderer);

  // Use our shader program
  shader_bind(t_obj->shader);

  texture_bind(t_obj->texture, 0);

  {
    mat4 model;
    glm_translate_make(model, (vec3){0.0f, 0.0f, 0.0f});
    glm_translate(model,
                  (vec3){t_obj->value_x, t_obj->value_y, t_obj->value_z});

    mat4 mvp;
    glm_mat4_mul(t_obj->proj, t_obj->view, mvp);
    glm_mat4_mul(mvp, model, mvp);

    shader_uniform_set_mat4f(t_obj->shader, "u_MVP", mvp);

    renderer_draw(t_obj->renderer, t_obj->va, t_obj->vb, t_obj->shader);
  }

  vertex_array_unbind();
}

static void on_ui_render(void *obj, void *context) {
  TextureObj *t_obj = (TextureObj *)obj;

  /* custom widget pixel width */
  nk_layout_row_begin(context, NK_STATIC, 25, 3);
  {
    nk_layout_row_push(context, 20);
    nk_label(context, "X:", NK_TEXT_LEFT);
    nk_layout_row_push(context, 30);
    char val[10];
    gcvt(t_obj->value_x, 9, val);
    nk_label(context, val, NK_TEXT_LEFT);
    nk_layout_row_push(context, 110);
    nk_slider_float(context, -100.0f, &t_obj->value_x, 100.0f, 1.0f);
  }
  nk_layout_row_end(context);

  /* custom widget pixel width */
  nk_layout_row_begin(context, NK_STATIC, 25, 3);
  {
    nk_layout_row_push(context, 20);
    nk_label(context, "Y:", NK_TEXT_LEFT);
    nk_layout_row_push(context, 30);
    char val[10];
    gcvt(t_obj->value_y, 9, val);
    nk_label(context, val, NK_TEXT_LEFT);
    nk_layout_row_push(context, 110);
    nk_slider_float(context, -100.0f, &t_obj->value_y, 100.0f, 1.0f);
  }
  nk_layout_row_end(context);

  /* custom widget pixel width */
  nk_layout_row_begin(context, NK_STATIC, 25, 3);
  {
    nk_layout_row_push(context, 20);
    nk_label(context, "Z:", NK_TEXT_LEFT);
    nk_layout_row_push(context, 30);
    char val[10];
    gcvt(t_obj->value_z, 9, val);
    nk_label(context, val, NK_TEXT_LEFT);
    nk_layout_row_push(context, 110);
    nk_slider_float(context, -1.0f, &t_obj->value_z, 1.0f, 1.0f);
  }
  nk_layout_row_end(context);
}

static void on_free(void *scene) {
  printf("Clear Color On Free\n");
  Scene *scene_p = (Scene *)scene;
  TextureObj *obj = (TextureObj *)scene_p->obj;

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

Scene *scene_texture_init() {
  Scene *scene = malloc(sizeof(Scene));

  scene->on_update = &on_update;
  scene->on_render = &on_render;
  scene->on_ui_render = &on_ui_render;
  scene->on_free = &on_free;

  TextureObj *obj = malloc(sizeof(TextureObj));
  scene->obj = obj;

  obj->value_x = 0.0f;
  obj->value_y = 0.0f;
  obj->value_z = 0.0f;

  obj->renderer = renderer_create();

  renderer_set_clear_color(obj->renderer, (float[]){0.2f, 0.3f, 0.3f, 1.0f});

  obj->shader = shader_create("resources/shaders/texture.glsl");
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

  glm_ortho(0.0f, WINDOW_WIDTH, 0.0f, WINDOW_HEIGHT, -1.0f, 1.0f, obj->proj);

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
