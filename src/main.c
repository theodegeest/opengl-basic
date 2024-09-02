#include "../include/glad/glad.h"
#include "scenes/scene.h"
#include "scenes/scene_batch_rendering.h"
#include "scenes/scene_clear_color.h"
#include "scenes/scene_color.h"
#include "scenes/scene_cube.h"
#include "scenes/scene_empty.h"
#include "scenes/scene_texture.h"
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
  Scene *(*init_function)();
} ScenePair;

// Function to handle key events
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

struct ui_args {
  const char **scene_options;
  int number_of_scenes;
  int *selected_scene_index;
  Scene *scene;
};

static void on_ui_function(struct nk_context *context, void *args) {
  struct ui_args *args_obj = (struct ui_args *)args;

  nk_layout_row_dynamic(context, 30, 1);
  nk_combobox(context, args_obj->scene_options, args_obj->number_of_scenes, args_obj->selected_scene_index, 20,
              (struct nk_vec2){200, 100});

  scene_on_ui_render(args_obj->scene, context);
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

  // Scene *scene = scene_texture_init();

  struct timespec last_time, current_time;
  double delta_time = 0;
  double fps = 0;
  float r = 0.0f;
  float r_inc = 0.002f;

  ScenePair scene_pairs[] = {
      {"Empty", &scene_empty_init},
      {"Clear Color", &scene_clear_color_init},
      {"Color", &scene_color_init},
      {"Texture", &scene_texture_init},
      {"Batch Rendering", &scene_batch_rendering_init},
      {"Cube", &scene_cube_init},
  };

  Scene *scene = scene_pairs[0].init_function();

  int number_of_scenes = sizeof(scene_pairs) / sizeof(ScenePair);

  const char *scene_options[number_of_scenes];
  for (int i = 0; i < number_of_scenes; i++) {
    scene_options[i] = scene_pairs[i].name;
  }

  // const char *scene_options[] = {
  //     "Clear Color", "Texture", "Empty"};      // The options we want to
  //     display
  static int selected_scene_index = 0; // Selected item index
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

    scene_on_update(scene, delta_time);

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    scene_on_render(scene);

    // Display the framerate (for debugging)
    // printf("FPS: %.2f\n", fps);

    struct ui_args args;
    args.scene_options = scene_options;
    args.number_of_scenes = number_of_scenes;
    args.selected_scene_index = &selected_scene_index;
    args.scene = scene;

    ui_draw_gui(ui, scene, fps, &on_ui_function, &args);

    if (selected_scene_index != previous_selected) {
      scene_on_free(scene);
      scene = scene_pairs[selected_scene_index].init_function();
      previous_selected = selected_scene_index;
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

  scene_on_free(scene);

  ui_free(ui);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
