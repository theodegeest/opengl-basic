#include "test_clear_color.h"
#include "../../include/glad/glad.h"
#include "../debug.h"
#include "../renderer.h"
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../include/Nuklear/nuklear.h"

typedef struct {
  struct nk_colorf clear_color;
  Renderer *renderer;
} ClearColorObj;

static void on_update(void *obj, float delta_time) {
  // printf("Clear Color On Update\n");
}

static void on_render(void *obj) {
  // printf("Clear Color On Render\n");
  ClearColorObj *c_obj = (ClearColorObj *)obj;
  renderer_set_clear_color(c_obj->renderer, &c_obj->clear_color.r);
  // GLCall(glClearColor(c_obj->clear_color.r, c_obj->clear_color.g,
  //                     c_obj->clear_color.b, c_obj->clear_color.a));
  // GLCall(glClear(GL_COLOR_BUFFER_BIT));
  renderer_clear(c_obj->renderer);
}

static void on_ui_render(void *obj, void *context) {
  ClearColorObj *c_obj = (ClearColorObj *)obj;

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

static void on_free(void *test) {
  printf("Clear Color On Free\n");
  Test *test_p = (Test *)test;
  ClearColorObj *obj = (ClearColorObj *)test_p->obj;

  renderer_free(obj->renderer);
  free(obj);
  free(test);
}

Test *test_clear_color_init() {
  Test *test = malloc(sizeof(Test));

  test->on_update = &on_update;
  test->on_render = &on_render;
  test->on_ui_render = &on_ui_render;
  test->on_free = &on_free;

  ClearColorObj *obj = malloc(sizeof(ClearColorObj));
  test->obj = obj;

  obj->clear_color = (struct nk_colorf){0.2f, 0.3f, 0.3f, 1.0f};

  obj->renderer = renderer_create();

  printf("Clear Color Init\n");

  return test;
}
