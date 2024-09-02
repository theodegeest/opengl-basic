#ifndef UI_H_
#define UI_H_

#include "../../include/Nuklear/nuklear.h"
#include "../tests/test.h"
#include <GLFW/glfw3.h>

typedef struct {
  struct nk_context *context;
} UI;

UI ui_init(GLFWwindow *window);
void ui_draw_gui(UI ui, Test *test, float fps, void (on_ui_function)(struct nk_context *context, void *args), void *args);

void ui_render(UI ui);

void ui_free(UI ui);

#endif // !UI_H_
