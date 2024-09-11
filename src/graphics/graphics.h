#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <GLFW/glfw3.h>

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 1000

GLFWwindow *graphics_init(int vsync);
void graphics_free(GLFWwindow *window);

#endif // !GRAPHICS_H_
