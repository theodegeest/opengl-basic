#ifndef UI_H_
#define UI_H_

#include "../../include/Nuklear/nuklear.h"
#include "../scenes/scene.h"
#include <GLFW/glfw3.h>

struct perf_values {
  double frame_time;
  double fps;
  double update_time;
  double scene_render_time;
  double ui_render_time;
};

typedef struct {
  struct nk_context *context;
  struct perf_values perf_values;
} UI;

UI ui_init(GLFWwindow *window);
void ui_draw_gui(UI ui, Scene *scene, void (on_ui_function)(struct nk_context *context, void *args), void *args);

void ui_render(UI ui);

void ui_free(UI ui);

#endif // !UI_H_
