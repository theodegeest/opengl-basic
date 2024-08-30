#include "../include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "index_buffer.h"
#include "renderer.h"
#include "shader.h"
#include "texture.h"
#include "vertex_array.h"
#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"
#include "debug.h"


// Function to handle key events
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
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

  // Load OpenGL function pointers with GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    fprintf(stderr, "Failed to initialize GLAD\n");
    return -1;
  }

  const GLubyte *gl_renderer = glGetString(GL_RENDERER); // get the GPU renderer
  const GLubyte *version = glGetString(GL_VERSION); // get the OpenGL version

  printf("Renderer: %s\n", gl_renderer);
  printf("OpenGL version supported %s\n\n", version);

  // Set a clear color to distinguish the triangle
  GLCall(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));

  // Set the key callback
  glfwSetKeyCallback(window, key_callback);

  Shader *shader = shader_create("resources/shaders/basic.glsl");
  shader_bind(shader);

  // Define the vertices for a triangle
  float vertices[] = {-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.5f, -0.5f,
                      0.0f,  1.0f,  0.0f, 0.5f, 0.5f, 0.0f, 1.0f,
                      1.0f,  -0.5f, 0.5f, 0.0f, 0.0f, 1.0f};

  unsigned int indices[] = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  GLCall(glEnable(GL_BLEND));
  GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // FIX:

  // Create and bind a Vertex Array Object
  VertexArray *va = vertex_array_create();

  // Create and bind a Vertex Buffer Object
  VertexBuffer *vb = vertex_buffer_create(vertices, sizeof(vertices));

  VertexBufferLayout *layout = vertex_buffer_layout_create();
  vertex_buffer_layout_push_float(layout, 3);
  vertex_buffer_layout_push_float(layout, 2);
  vertex_array_add_buffer(va, vb, layout);
  // glCheckError();

  // Create and bind a Index Buffer Object
  IndexBuffer *ib = index_buffer_create(indices, 6);

  // glClearError();
  Texture *texture = texture_create("resources/textures/opengl.png");
  // glCheckError();
  texture_bind(texture, 0);
  // glClearError();
  shader_uniform_set_1i(shader, "u_Texture", 0);
  // glCheckError();


  // Unbind the VBO and VAO
  vertex_array_unbind();
  vertex_buffer_unbind();
  index_buffer_unbind();

  float r = 0.0f;
  float r_inc = 0.002f;

  Renderer *renderer = renderer_create();

  // Rendering loop
  while (!glfwWindowShouldClose(window)) {
    r += r_inc;
    if (r > 1) {
      r = 0.0f;
    }
    // Clear the screen
    // glClear(GL_COLOR_BUFFER_BIT);
    renderer_clear(renderer);

    // Use our shader program
    shader_bind(shader);

    shader_uniform_set_4f(shader, "u_Color", r, 0.3f, 0.8f, 1.0f);
    // glCheckError();

    // Draw the triangle
    // vertex_array_bind(va);

    renderer_draw(renderer, va, ib, shader);
    // glClearError();
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    // glCheckError();

    vertex_array_unbind();

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }

  // Clean up
  vertex_array_free(va);
  vertex_buffer_free(vb);
  index_buffer_free(ib);
  vertex_buffer_layout_free(layout);

  shader_free(shader);
  renderer_free(renderer);
  texture_free(texture);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
