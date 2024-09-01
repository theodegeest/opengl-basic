#ifndef TEST_H_
#define TEST_H_

typedef struct {
  void (*on_update)(void *obj, float delta_time);
  void (*on_render)(void *obj);
  void (*on_ui_render)(void *obj, void *context);
  void (*on_free)(void *test);
  void *obj;
} Test;

void test_on_update(Test *test, float delta_time);
void test_on_render(Test *test);
void test_on_ui_render(Test *test, void *context);
void test_on_free(Test *test);

#endif // !TEST_H_
