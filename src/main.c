#include "../include/glad/glad.h"
#include "scenes/scene.h"
#include "scenes/scene_batch_rendering.h"
#include "scenes/scene_clear_color.h"
#include "scenes/scene_color.h"
#include "scenes/scene_cube.h"
#include "scenes/scene_empty.h"
#include "scenes/scene_pixel_sim.h"
#include "scenes/scene_shapes.h"
#include "scenes/scene_texture.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

#include <cglm/cglm.h>
#include <time.h>

#include "./graphics/debug.h"
#include "./graphics/graphics.h"
#include "./graphics/ui.h"
#include "utils/timer.h"

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
  nk_combobox(context, args_obj->scene_options, args_obj->number_of_scenes,
              args_obj->selected_scene_index, 20, (struct nk_vec2){200, 100});

  scene_on_ui_render(args_obj->scene, context);
}

int main(void) {
#ifdef VSYNC
  GLFWwindow *window = graphics_init(1);
#else
  GLFWwindow *window = graphics_init(0);
#endif /* ifdef VSYNC */
  UI ui = ui_init(window);

  ScenePair scene_pairs[] = {
      {"Pixel Sim", &scene_pixel_sim_init},
      {"Empty", &scene_empty_init},
      {"Clear Color", &scene_clear_color_init},
      {"Color", &scene_color_init},
      {"Shapes", &scene_shapes_init},
      {"Texture", &scene_texture_init},
      {"Batch Rendering", &scene_batch_rendering_init},
      {"Cube", &scene_cube_init},
      // {"Pixel Sim", &scene_pixel_sim_init},
  };

  Scene *scene = scene_pairs[0].init_function();

  int number_of_scenes = sizeof(scene_pairs) / sizeof(ScenePair);

  int selected_scene_index = 0;
  int previous_selected = 0;
  const char *scene_options[number_of_scenes];

  for (int i = 0; i < number_of_scenes; i++) {
    scene_options[i] = scene_pairs[i].name;
  }

  Timer *frame_time = timer_init();
  Timer *update_time = timer_init();
  Timer *scene_render_time = timer_init();
  Timer *ui_render_time = timer_init();

  struct timespec last_time, current_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);
  double delta_time = 0;

  // Rendering loop
  while (!glfwWindowShouldClose(window)) {
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    delta_time = (current_time.tv_sec - last_time.tv_sec) +
                 (current_time.tv_nsec - last_time.tv_nsec) / 1000000.0;
    last_time = current_time;

    timer_start(frame_time);

    timer_start(update_time);
    scene_on_update(scene, delta_time, window);
    timer_stop(update_time);

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    timer_start(scene_render_time);
    scene_on_render(scene);
    timer_stop(scene_render_time);

    struct ui_args args;
    args.scene_options = scene_options;
    args.number_of_scenes = number_of_scenes;
    args.selected_scene_index = &selected_scene_index;
    args.scene = scene;

    timer_start(ui_render_time);
    ui_draw_gui(ui, scene, &on_ui_function, &args);

    if (selected_scene_index != previous_selected) {
      scene_on_free(scene);
      scene = scene_pairs[selected_scene_index].init_function();
      previous_selected = selected_scene_index;
    }

    ui_render(ui);
    timer_stop(ui_render_time);
    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();

    ui.perf_values.fps = 1.0 / (timer_stop(frame_time) / 1000);
    ui.perf_values.frame_time = timer_elapsed(frame_time);
    ui.perf_values.update_time = timer_elapsed(update_time);
    ui.perf_values.scene_render_time = timer_elapsed(scene_render_time);
    ui.perf_values.ui_render_time = timer_elapsed(ui_render_time);

    // printf("%f\n", ui.perf_values.fps);

    // int target_fps = 2;
    // long target_frame_time = 1000000000 / target_fps;
    // struct timespec delay;
    // delay.tv_sec = target_frame_time / 1000000000L;
    // delay.tv_nsec = target_frame_time % 1000000000L;
    // nanosleep(&delay, NULL);
  }

  timer_free(frame_time);
  timer_free(update_time);
  timer_free(scene_render_time);

  // Clean up

  scene_on_free(scene);

  ui_free(ui);

  graphics_free(window);

  return 0;
}
