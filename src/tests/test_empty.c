#include "test_empty.h"
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
  Renderer *renderer;
} ClearColorObj;

static void on_update(void *obj, float delta_time) {
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

static void on_free(void *test) {
  printf("Clear Color On Free\n");
  Test *test_p = (Test *)test;
  ClearColorObj *obj = (ClearColorObj *)test_p->obj;

  renderer_free(obj->renderer);
  free(obj);
  free(test);
}

Test *test_empty_init() {
  Test *test = malloc(sizeof(Test));

  test->on_update = &on_update;
  test->on_render = &on_render;
  test->on_ui_render = &on_ui_render;
  test->on_free = &on_free;

  ClearColorObj *obj = malloc(sizeof(ClearColorObj));
  test->obj = obj;

  obj->renderer = renderer_create();

  renderer_set_clear_color(obj->renderer, (float[]){0.2f, 0.3f, 0.3f, 1.0f});

  printf("Clear Color Init\n");

  return test;
}
