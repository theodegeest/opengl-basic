#include "../../include/glad/glad.h"
#include "scene_empty.h"
#include "../graphics/renderer.h"
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../include/Nuklear/nuklear.h"

typedef struct {
  Renderer *renderer;
} ClearColorObj;

static void on_update(void *obj, float delta_time, GLFWwindow *window) {
  // printf("Clear Color On Update\n");
}

static void on_render(void *obj) {
  ClearColorObj *c_obj = (ClearColorObj *)obj;
  renderer_clear(c_obj->renderer);
  // GLCall(glClear(GL_COLOR_BUFFER_BIT));
}

static void on_ui_render(void *obj, void *context) {
  // ClearColorObj *c_obj = (ClearColorObj *)obj;
}

static void on_free(void *scene) {
  printf("Clear Color On Free\n");
  Scene *scene_p = (Scene *)scene;
  ClearColorObj *obj = (ClearColorObj *)scene_p->obj;

  renderer_free(obj->renderer);
  free(obj);
  free(scene);
}

Scene *scene_empty_init() {
  Scene *scene = malloc(sizeof(Scene));

  scene->on_update = &on_update;
  scene->on_render = &on_render;
  scene->on_ui_render = &on_ui_render;
  scene->on_free = &on_free;

  ClearColorObj *obj = malloc(sizeof(ClearColorObj));
  scene->obj = obj;

  obj->renderer = renderer_create();

  renderer_set_clear_color(obj->renderer, (float[]){0.2f, 0.3f, 0.3f, 1.0f});

  printf("Clear Color Init\n");

  return scene;
}
