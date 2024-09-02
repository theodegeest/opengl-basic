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
#include "./graphics/graphics.h"

typedef struct {
  const char *name;
  Scene *(*init_function)();
} ScenePair;

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
  GLFWwindow *window = graphics_init(0);
  UI ui = ui_init(window);

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

  int selected_scene_index = 0;
  int previous_selected = 0;
  const char *scene_options[number_of_scenes];

  for (int i = 0; i < number_of_scenes; i++) {
    scene_options[i] = scene_pairs[i].name;
  }


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
