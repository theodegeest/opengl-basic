#include "../../include/glad/glad.h"
#include <GLFW/glfw3.h>

//----------------------nk-------------------------
// Here we define everything needed to set up the library with glfw.
// Note that nuklear_glfw_gl3.h should be included after nuklear.h.
#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION

// #include "../include/Nuklear/nuklear.h"
#include "ui.h"
//
#include "../../include/Nuklear/demo/glfw_opengl4/nuklear_glfw_gl4.h"
// #include <nuklear\demo\glfw_opengl3\nuklear_glfw_gl3.h>
//----------------------------------------------------

UI ui_init(GLFWwindow *window) {
  //-------------------------nk-------------------------------
  // Here we set up the context for nuklear which contains info about the
  // styles, fonts etc about the GUI We also set up the font here
  struct nk_context *context =
      nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER,
                    MAX_ELEMENT_BUFFER);
  // set up font
  struct nk_font_atlas *atlas;
  nk_glfw3_font_stash_begin(&atlas);
  nk_glfw3_font_stash_end();
  //----------------------------------------------------------

  UI ui;
  ui.context = context;

  return ui;
}

void ui_draw_gui(UI ui, Scene *scene,
                 void(on_ui_function)(struct nk_context *context, void *args),
                 void *args) {
  struct nk_context *context = ui.context;
  //--------------------------nk------------------------------
  // create a new frame for every iteration of the loop
  // here we set up the nk_window
  nk_glfw3_new_frame();

  float scene_time =
      ui.perf_values.update_time + ui.perf_values.scene_render_time;

  if (nk_begin(context, "Performance", nk_rect(0, 0, 220, 300),
               NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE |
                   NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE)) {
    nk_layout_row_static(context, 15, 110, 2);
    char string[20];

    nk_label(context, "fps:", NK_TEXT_LEFT);
    gcvt(ui.perf_values.fps, 8, string);
    nk_label(context, string, NK_TEXT_LEFT);

    nk_label(context, "ms/frame:", NK_TEXT_LEFT);
    gcvt(ui.perf_values.frame_time, 8, string);
    nk_label(context, string, NK_TEXT_LEFT);

    const float update_widths[] = {0.4f, 0.15f, 0.1f, 0.4025f};
    nk_layout_row(context, NK_DYNAMIC, 15, 4, update_widths);
    // nk_layout_row_static(context, 15, 50, 4);

    nk_label(context, "update time:", NK_TEXT_LEFT);
    gcvt((int)((ui.perf_values.update_time / scene_time) * 100),
         8, string);
    nk_label(context, string, NK_TEXT_RIGHT);
    nk_label(context, "%, ", NK_TEXT_LEFT);
    gcvt(ui.perf_values.update_time, 8, string);
    nk_label(context, string, NK_TEXT_LEFT);

    const float render_widths[] = {0.4f, 0.15f, 0.1f, 0.4025f};
    nk_layout_row(context, NK_DYNAMIC, 15, 4, render_widths);
    // nk_layout_row_static(context, 15, 110, 2);

    nk_label(context, "render time:", NK_TEXT_LEFT);
    gcvt((int)((ui.perf_values.scene_render_time / scene_time) *
               100),
         8, string);
    nk_label(context, string, NK_TEXT_RIGHT);
    nk_label(context, "%, ", NK_TEXT_LEFT);
    gcvt(ui.perf_values.scene_render_time, 8, string);
    nk_label(context, string, NK_TEXT_LEFT);

    nk_layout_row_static(context, 15, 110, 2);
    nk_label(context, "ui render time:", NK_TEXT_LEFT);
    gcvt(ui.perf_values.ui_render_time, 8, string);
    nk_label(context, string, NK_TEXT_LEFT);

    on_ui_function(context, args);
  }
  nk_end(context);
  //------------------------------------------------------------
}

void ui_render(UI ui) {
  //-------------------------nk------------------------------------
  // don't forget to draw your window!
  nk_glfw3_render(NK_ANTI_ALIASING_ON);
  //---------------------------------------------------------------
}

void ui_free(UI ui) {
  //-------------------------nk------------------------------------
  nk_glfw3_shutdown();
  //-------------------------------------------------------------
}
