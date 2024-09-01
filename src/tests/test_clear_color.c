#include "test_clear_color.h"
#include "../../include/glad/glad.h"
#include "../debug.h"
#include <stdio.h>
#include <stdlib.h>

#include "../../include/Nuklear/nuklear.h"

typedef struct {
  struct nk_colorf clear_color;
} ClearColorObj;

static void on_update(void *obj, float delta_time) {
  // printf("Clear Color On Update\n");
}

static void on_render(void *obj) {
  // printf("Clear Color On Render\n");
  ClearColorObj *clear_color_obj = (ClearColorObj *)obj;
  GLCall(glClearColor(
      clear_color_obj->clear_color.r, clear_color_obj->clear_color.g,
      clear_color_obj->clear_color.b, clear_color_obj->clear_color.a));
  GLCall(glClear(GL_COLOR_BUFFER_BIT))
}

static void on_ui_render(void *obj, void *context) {
  ClearColorObj *clear_color_obj = (ClearColorObj *)obj;

  nk_layout_row_push(context, 120);
  nk_label(context, "Clear Color:", NK_TEXT_LEFT);
  nk_layout_row_begin(context, NK_STATIC, 100, 1);
  {
    nk_layout_row_push(context, 150);
    clear_color_obj->clear_color =
        nk_color_picker(context, clear_color_obj->clear_color, NK_RGBA);
  }
  nk_layout_row_end(context);
  nk_layout_row_push(context, 120);
  nk_label(context, "UI Render", NK_TEXT_LEFT);
}

static void on_free(void *test) {
  printf("Clear Color On Free\n");
  free(((Test *)test)->obj);
  free(test);
}

Test *test_clear_color_init() {
  Test *test = malloc(sizeof(Test));

  test->on_update = &on_update;
  test->on_render = &on_render;
  test->on_ui_render = &on_ui_render;
  test->on_free = &on_free;

  ClearColorObj *obj = malloc(sizeof(ClearColorObj));

  obj->clear_color = (struct nk_colorf){0.2f, 0.3f, 0.3f, 1.0f};

  test->obj = obj;

  printf("Clear Color Init\n");

  return test;
}
