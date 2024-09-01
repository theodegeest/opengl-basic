#include "test.h"

void test_on_update(Test *test, float delta_time) {
  test->on_update(test->obj, delta_time);
}

void test_on_render(Test *test) {
  test->on_render(test->obj);
}

void test_on_ui_render(Test *test, void *context) {
  test->on_ui_render(test->obj, context);
}

void test_on_free(Test *test) {
  test->on_free(test);
}
