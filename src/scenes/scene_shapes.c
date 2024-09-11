#include "scene_shapes.h"
#include "../../include/glad/glad.h"
#include "../graphics/graphics.h"
#include "../graphics/index_buffer.h"
#include "../graphics/renderer.h"
#include "../graphics/shader.h"
#include "../graphics/vertex_array.h"
#include "../mesh/shape.h"
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
} ShapesObj;

static void on_update(void *obj, float delta_time) {
  ShapesObj *s_obj = (ShapesObj *)obj;
  // printf("Clear shapes On Update\n");

  // Create Quads
  vertex_buffer_clear(s_obj->vb);

  // Quad left_quad = quad_create(
  //     350.0f, 250.0f, 0.0f, 100.0f, 100.0f, 0.0f,
  //     (Color){s_obj->color.r, s_obj->color.g, s_obj->color.b,
  //     s_obj->color.a}, 0);
  ShapeBox box1 = shape_box_create(
      350.0f, 250.0f, 100.0f, 100.0f, 10.0f,
      (Color){s_obj->color.r, s_obj->color.g, s_obj->color.b, s_obj->color.a},
      0);

  ShapeBox box2 = shape_box_create(
      500.0f, 250.0f, 100.0f, 100.0f, 2.0f,
      (Color){s_obj->color.r, s_obj->color.g, s_obj->color.b, s_obj->color.a},
      0);

  vertex_buffer_push(s_obj->vb, (Vertex *)&box1, SHAPE_BOX_NUMBER_OF_VERTICES);
  vertex_buffer_push(s_obj->vb, (Vertex *)&box2, SHAPE_BOX_NUMBER_OF_VERTICES);

  // printf("%f\n", s_obj->vb->buffer[4].ld.pos.x);
  // printf("%f\n", s_obj->vb->buffer[100].ld.pos.x);

  vertex_buffer_flush(s_obj->vb);
}

static void on_render(void *obj) {
  // printf("Clear shapes On Render\n");
  ShapesObj *s_obj = (ShapesObj *)obj;
  renderer_clear(s_obj->renderer);

  // Use our shader program
  shader_bind(s_obj->shader);

  {
    mat4 model;
    glm_translate_make(model, (vec3){0.0f, 0.0f, 0.0f});

    mat4 mvp;
    glm_mat4_mul(s_obj->proj, s_obj->view, mvp);
    glm_mat4_mul(mvp, model, mvp);

    shader_uniform_set_mat4f(s_obj->shader, "u_MVP", mvp);

    renderer_draw(s_obj->renderer, s_obj->va, s_obj->vb, s_obj->shader);
  }

  // {
  //   mat4 model;
  //   glm_translate_make(model, (vec3){300.0f, 100.0f, 0.0f});
  //   glm_translate(
  //       model, (vec3){s_obj->value_x * 2, s_obj->value_y * 2,
  //       s_obj->value_z});
  //
  //   mat4 mvp;
  //   glm_mat4_mul(s_obj->proj, s_obj->view, mvp);
  //   glm_mat4_mul(mvp, model, mvp);
  //
  //   shader_uniform_set_mat4f(s_obj->shader, "u_MVP", mvp);
  //
  //   renderer_draw(s_obj->renderer, s_obj->va, s_obj->ib, s_obj->shader);
  // }

  vertex_array_unbind();
}

static void on_ui_render(void *obj, void *context) {
  ShapesObj *s_obj = (ShapesObj *)obj;

  nk_layout_row_begin(context, NK_STATIC, 20, 1);
  {
    nk_layout_row_push(context, 110);
    nk_label(context, "Color:", NK_TEXT_LEFT);
  }
  nk_layout_row_end(context);

  nk_layout_row_begin(context, NK_STATIC, 90, 1);
  {
    nk_layout_row_push(context, 150);
    s_obj->color = nk_color_picker(context, s_obj->color, NK_RGBA);
  }
  nk_layout_row_end(context);
}

static void on_free(void *scene) {
  printf("Clear shapes On Free\n");
  Scene *scene_p = (Scene *)scene;
  ShapesObj *obj = (ShapesObj *)scene_p->obj;

  vertex_array_free(obj->va);
  vertex_buffer_free(obj->vb);
  index_buffer_free(obj->ib);
  vertex_buffer_layout_free(obj->layout);
  shader_free(obj->shader);
  renderer_free(obj->renderer);
  free(obj);
  free(scene);
}

Scene *scene_shapes_init() {
  Scene *scene = malloc(sizeof(Scene));

  scene->on_update = &on_update;
  scene->on_render = &on_render;
  scene->on_ui_render = &on_ui_render;
  scene->on_free = &on_free;

  ShapesObj *obj = malloc(sizeof(ShapesObj));
  scene->obj = obj;

  obj->color = (struct nk_colorf){1.0f, 0.0f, 0.0f, 1.0f};

  obj->renderer = renderer_create();

  renderer_set_clear_color(obj->renderer, (float[]){0.2f, 0.3f, 0.3f, 1.0f});

  obj->shader = shader_create("resources/shaders/shapes.glsl");
  shader_bind(obj->shader);

  // Create and bind a Vertex Array Object
  obj->va = vertex_array_create();

  int total_number_of_vertices = 0;
  total_number_of_vertices += SHAPE_BOX_NUMBER_OF_VERTICES * 2;

  // Create and bind a Vertex Buffer Object
  obj->vb = vertex_buffer_create(total_number_of_vertices);

  obj->layout = vertex_buffer_layout_create();
  vertex_buffer_layout_push_float(obj->layout, 3);
  vertex_buffer_layout_push_float(obj->layout, 4);
  vertex_buffer_layout_push_float(obj->layout, 2);
  vertex_buffer_layout_push_float(obj->layout, 1);
  vertex_array_add_buffer(obj->va, obj->vb, obj->layout);

  // Create and bind a Index Buffer Object
  // obj->ib = index_buffer_create(indices, 6 * 3);
  obj->ib = index_buffer_create_quad(total_number_of_vertices *
                                     QUAD_NUMBER_OF_VERTICES);

  glm_ortho(0.0f, WINDOW_WIDTH, 0.0f, WINDOW_HEIGHT, -1.0f, 1.0f, obj->proj);

  glm_mat4_identity(obj->view);
  glm_translate(obj->view, (vec3){0.0f, 0.0f, 0.0f});

  // Unbind the VBO and VAO
  vertex_array_unbind();
  vertex_buffer_unbind();
  index_buffer_unbind();

  printf("Clear shapes Init\n");

  return scene;
}
