#include "../include/glad/glad.h"
#include "tests/test.h"
#include "tests/test_clear_color.h"
#include "tests/test_empty.h"
#include "tests/test_texture.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

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

#include "../include/Nuklear/nuklear.h"
//
#include "../include/Nuklear/demo/glfw_opengl4/nuklear_glfw_gl4.h"
// #include <nuklear\demo\glfw_opengl3\nuklear_glfw_gl3.h>
//----------------------------------------------------

#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "debug.h"

// Function to handle key events
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void) {
  // Initialize GLFW
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  // Set OpenGL version to 3.3 and profile to core
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a windowed mode window and its OpenGL context
  GLFWwindow *window =
      glfwCreateWindow(800, 600, "OpenGL with GLAD", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);

  // Load OpenGL function pointers with GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    fprintf(stderr, "Failed to initialize GLAD\n");
    return -1;
  }

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

  const GLubyte *gl_renderer = glGetString(GL_RENDERER); // get the GPU renderer
  const GLubyte *version = glGetString(GL_VERSION); // get the OpenGL version

  printf("Renderer: %s\n", gl_renderer);
  printf("OpenGL version supported %s\n\n", version);

  // Set a clear color to distinguish the triangle
  GLCall(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));

  // Set the key callback
  glfwSetKeyCallback(window, key_callback);

  Test *test = test_clear_color_init();
  // Test *test = test_texture_init();

  struct timespec last_time, current_time;
  double delta_time = 0;
  double fps = 0;
  float r = 0.0f;
  float r_inc = 0.002f;

  static const char *test_options[] = {
      "Clear Color", "Texture", "Empty"};      // The options we want to display
  static int selected_test_index = 0; // Selected item index
  int previous_selected = 0;

  // Initialize lastTime using CLOCK_MONOTONIC
  clock_gettime(CLOCK_MONOTONIC, &last_time);

  // Rendering loop
  while (!glfwWindowShouldClose(window)) {
    // Get the current time using CLOCK_MONOTONIC
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    // Calculate delta time in seconds
    delta_time = (current_time.tv_sec - last_time.tv_sec) +
                 (current_time.tv_nsec - last_time.tv_nsec) / 1000000000.0;

    // Update lastTime for the next loop iteration
    last_time = current_time;

    // Calculate the framerate
    if (delta_time > 0) {
      fps = 1.0 / delta_time;
    }

    test_on_update(test, delta_time);

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    test_on_render(test);

    // Display the framerate (for debugging)
    // printf("FPS: %.2f\n", fps);

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

      nk_layout_row_dynamic(context, 30, 1);
      nk_combobox(context, test_options, sizeof(test_options) / sizeof(char *), &selected_test_index, 20,
                  (struct nk_vec2){200, 100});

      test_on_ui_render(test, context);
    }
    nk_end(context);
    //------------------------------------------------------------

    if (selected_test_index != previous_selected) {
      switch (selected_test_index) {
      case 0:
        test_on_free(test);
        test = test_clear_color_init();
        break;
      case 1:
        test_on_free(test);
        test = test_texture_init();
        break;
      case 2:
        test_on_free(test);
        test = test_empty_init();
        break;
      default:
        printf("Unknown selected item");
        break;
      }

      previous_selected = selected_test_index;
    }

    r += r_inc;
    if (r > 1) {
      r = 0.0f;
    }
    // Clear the screen
    // glClear(GL_COLOR_BUFFER_BIT);
    // renderer_clear(renderer);

    //-------------------------nk------------------------------------
    // don't forget to draw your window!
    nk_glfw3_render(NK_ANTI_ALIASING_ON);
    //---------------------------------------------------------------
    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }

  // Clean up

  test_on_free(test);

  //-------------------------nk------------------------------------
  nk_glfw3_shutdown();
  //-------------------------------------------------------------

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
