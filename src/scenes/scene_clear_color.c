#include "scene_clear_color.h"
#include "../../include/glad/glad.h"
#include "../graphics/renderer.h"
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../include/Nuklear/nuklear.h"

typedef struct {
  struct nk_colorf clear_color;
  Renderer *renderer;
} BatchRenderingObj;

static void on_update(void *obj, float delta_time) {
  // printf("Clear Color On Update\n");
}

static void on_render(void *obj) {
  // printf("Clear Color On Render\n");
  BatchRenderingObj *c_obj = (BatchRenderingObj *)obj;
  renderer_set_clear_color(c_obj->renderer, &c_obj->clear_color.r);
  renderer_clear(c_obj->renderer);
}

static void on_ui_render(void *obj, void *context) {
  BatchRenderingObj *c_obj = (BatchRenderingObj *)obj;

  nk_layout_row_begin(context, NK_STATIC, 20, 1);
  {
    nk_layout_row_push(context, 110);
    nk_label(context, "Clear Color:", NK_TEXT_LEFT);
  }
  nk_layout_row_end(context);

  nk_layout_row_begin(context, NK_STATIC, 90, 1);
  {
    nk_layout_row_push(context, 150);
    c_obj->clear_color = nk_color_picker(context, c_obj->clear_color, NK_RGBA);
  }
  nk_layout_row_end(context);
}

static void on_free(void *scene) {
  printf("Clear Color On Free\n");
  Scene *scene_p = (Scene *)scene;
  BatchRenderingObj *obj = (BatchRenderingObj *)scene_p->obj;

  renderer_free(obj->renderer);
  free(obj);
  free(scene);
}

Scene *scene_clear_color_init() {
  Scene *scene = malloc(sizeof(Scene));

  scene->on_update = &on_update;
  scene->on_render = &on_render;
  scene->on_ui_render = &on_ui_render;
  scene->on_free = &on_free;

  BatchRenderingObj *obj = malloc(sizeof(BatchRenderingObj));
  scene->obj = obj;

  obj->clear_color = (struct nk_colorf){0.2f, 0.3f, 0.3f, 1.0f};

  obj->renderer = renderer_create();

  printf("Clear Color Init\n");

  return scene;
}
