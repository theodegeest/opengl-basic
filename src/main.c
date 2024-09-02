#include "../include/glad/glad.h"
#include "tests/test.h"
#include "tests/test_batch_rendering.h"
#include "tests/test_clear_color.h"
#include "tests/test_color.h"
#include "tests/test_cube.h"
#include "tests/test_empty.h"
#include "tests/test_texture.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "./graphics/debug.h"
#include "./graphics/ui.h"

typedef struct {
  const char *name;
  Test *(*init_function)();
} TestPair;

// Function to handle key events
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

struct ui_args {
  const char **test_options;
  int number_of_tests;
  int *selected_test_index;
  Test *test;
};

static void on_ui_function(struct nk_context *context, void *args) {
  struct ui_args *args_obj = (struct ui_args *)args;

  nk_layout_row_dynamic(context, 30, 1);
  nk_combobox(context, args_obj->test_options, args_obj->number_of_tests, args_obj->selected_test_index, 20,
              (struct nk_vec2){200, 100});

  test_on_ui_render(args_obj->test, context);
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

  UI ui = ui_init(window);

  const GLubyte *gl_renderer = glGetString(GL_RENDERER); // get the GPU renderer
  const GLubyte *version = glGetString(GL_VERSION); // get the OpenGL version

  printf("Renderer: %s\n", gl_renderer);
  printf("OpenGL version supported %s\n\n", version);

  // Set a clear color to distinguish the triangle
  GLCall(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));

  // Set the key callback
  glfwSetKeyCallback(window, key_callback);

  // Test *test = test_texture_init();

  struct timespec last_time, current_time;
  double delta_time = 0;
  double fps = 0;
  float r = 0.0f;
  float r_inc = 0.002f;

  TestPair test_pairs[] = {
      {"Empty", &test_empty_init},
      {"Clear Color", &test_clear_color_init},
      {"Color", &test_color_init},
      {"Texture", &test_texture_init},
      {"Batch Rendering", &test_batch_rendering_init},
      {"Cube", &test_cube_init},
  };

  Test *test = test_pairs[0].init_function();

  int number_of_tests = sizeof(test_pairs) / sizeof(TestPair);

  const char *test_options[number_of_tests];
  for (int i = 0; i < number_of_tests; i++) {
    test_options[i] = test_pairs[i].name;
  }

  // const char *test_options[] = {
  //     "Clear Color", "Texture", "Empty"};      // The options we want to
  //     display
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

    struct ui_args args;
    args.test_options = test_options;
    args.number_of_tests = number_of_tests;
    args.selected_test_index = &selected_test_index;
    args.test = test;

    ui_draw_gui(ui, test, fps, &on_ui_function, &args);

    if (selected_test_index != previous_selected) {
      test_on_free(test);
      test = test_pairs[selected_test_index].init_function();
      previous_selected = selected_test_index;
    }

    r += r_inc;
    if (r > 1) {
      r = 0.0f;
    }
    // Clear the screen
    // glClear(GL_COLOR_BUFFER_BIT);
    // renderer_clear(renderer);

    ui_render(ui);
    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }

  // Clean up

  test_on_free(test);

  ui_free(ui);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
