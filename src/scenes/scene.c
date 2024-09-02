#include "scene.h"

void scene_on_update(Scene *scene, float delta_time) {
  scene->on_update(scene->obj, delta_time);
}

void scene_on_render(Scene *scene) {
  scene->on_render(scene->obj);
}

void scene_on_ui_render(Scene *scene, void *context) {
  scene->on_ui_render(scene->obj, context);
}

void scene_on_free(Scene *scene) {
  scene->on_free(scene);
}
