#include "../../include/glad/glad.h"
#include "graphics.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>

// Function to handle key events
static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

GLFWwindow *graphics_init(int vsync) {
  // Initialize GLFW
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    exit(EXIT_FAILURE);
  }

  // Set OpenGL version to 3.3 and profile to core
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a windowed mode window and its OpenGL context
  GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                        "OpenGL with GLAD", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);
  if (!vsync) {
    glfwSwapInterval(0);
  }

  // Load OpenGL function pointers with GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    fprintf(stderr, "Failed to initialize GLAD\n");
    exit(EXIT_FAILURE);
  }

  const GLubyte *gl_renderer = glGetString(GL_RENDERER); // get the GPU renderer
  const GLubyte *version = glGetString(GL_VERSION); // get the OpenGL version

  printf("Renderer: %s\n", gl_renderer);
  printf("OpenGL version supported %s\n\n", version);

  // Set a clear color to distinguish the triangle
  GLCall(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));

  // Set the key callback
  glfwSetKeyCallback(window, key_callback);

  return window;
}

void graphics_free(GLFWwindow *window) {
  glfwDestroyWindow(window);
  glfwTerminate();
}
