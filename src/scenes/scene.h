#ifndef SCENE_H_
#define SCENE_H_

typedef struct {
  void (*on_update)(void *obj, float delta_time);
  void (*on_render)(void *obj);
  void (*on_ui_render)(void *obj, void *context);
  void (*on_free)(void *scene);
  void *obj;
} Scene;

void scene_on_update(Scene *scene, float delta_time);
void scene_on_render(Scene *scene);
void scene_on_ui_render(Scene *scene, void *context);
void scene_on_free(Scene *scene);

#endif // !SCENE_H_
