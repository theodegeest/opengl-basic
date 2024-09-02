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

  return (UI){context};
}

void ui_draw_gui(UI ui, Test *test, float fps,
                 void(on_ui_function)(struct nk_context *context, void *args),
                 void *args) {
  struct nk_context *context = ui.context;
  //--------------------------nk------------------------------
  // create a new frame for every iteration of the loop
  // here we set up the nk_window
  nk_glfw3_new_frame();

  if (nk_begin(context, "Nuklear Window", nk_rect(0, 0, 200, 300),
               NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE |
                   NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE)) {
    nk_layout_row_begin(context, NK_STATIC, 30, 2);
    {
      nk_layout_row_push(context, 30);
      nk_label(context, "fps:", NK_TEXT_LEFT);
      nk_layout_row_push(context, 50);
      char string_fps[11];
      gcvt(fps, 8, string_fps);
      nk_label(context, string_fps, NK_TEXT_LEFT);
    }
    nk_layout_row_end(context);

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
