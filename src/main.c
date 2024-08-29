#include "../include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
  char *vertexShaderSource;
  char *fragmentShaderSource;
} ShaderSources;

ShaderSources loadShadersFromFile(const char *filePath) {
  ShaderSources sources = {NULL, NULL};
  FILE *file = fopen(filePath, "r");

  if (!file) {
    fprintf(stderr, "Failed to open shader file: %s\n", filePath);
    return sources;
  }

  char *vertexShaderSource = NULL;
  char *fragmentShaderSource = NULL;

  size_t vertexLength = 0;
  size_t fragmentLength = 0;

  char line[256];
  enum { NONE, VERTEX, FRAGMENT } shaderType = NONE;

  while (fgets(line, sizeof(line), file)) {
    if (strncmp(line, "#shader", 7) == 0) {
      if (strstr(line, "vertex")) {
        shaderType = VERTEX;
      } else if (strstr(line, "fragment")) {
        shaderType = FRAGMENT;
      }
    } else {
      if (shaderType == VERTEX) {
        size_t lineLength = strlen(line);
        vertexShaderSource =
            realloc(vertexShaderSource, vertexLength + lineLength + 1);
        if (vertexShaderSource == NULL) {
          fprintf(stderr, "Failed to allocate memory for vertex shader\n");
          break;
        }
        memcpy(vertexShaderSource + vertexLength, line, lineLength + 1);
        vertexLength += lineLength;
      } else if (shaderType == FRAGMENT) {
        size_t lineLength = strlen(line);
        fragmentShaderSource =
            realloc(fragmentShaderSource, fragmentLength + lineLength + 1);
        if (fragmentShaderSource == NULL) {
          fprintf(stderr, "Failed to allocate memory for fragment shader\n");
          break;
        }
        memcpy(fragmentShaderSource + fragmentLength, line, lineLength + 1);
        fragmentLength += lineLength;
      }
    }
  }

  fclose(file);

  sources.vertexShaderSource = vertexShaderSource;
  sources.fragmentShaderSource = fragmentShaderSource;

  return sources;
}

// Function to compile shaders and check for errors
unsigned int compileShader(unsigned int type, const char *source) {
  unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  // Check for shader compile errors
  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    printf("ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
  }
  return shader;
}

unsigned int createShader(const char *vertexShaderSource,
                          const char *fragmentShaderSource) {
  // Build and compile our shader program
  unsigned int vertexShader =
      compileShader(GL_VERTEX_SHADER, vertexShaderSource);
  unsigned int fragmentShader =
      compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  // Link shaders into a program
  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  // Check for linking errors
  int success;
  char infoLog[512];
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
  }

  // Clean up shaders as they are now linked into our program and no longer
  // necessary
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

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

  const GLubyte *renderer = glGetString(GL_RENDERER); // get the GPU renderer
  const GLubyte *version = glGetString(GL_VERSION);   // get the OpenGL version

  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", version);

  // Set a clear color to distinguish the triangle
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  // Set the key callback
  glfwSetKeyCallback(window, key_callback);

  ShaderSources shaderSources =
      loadShadersFromFile("resources/shaders/basic.glsl");

  printf("Vertex shader source:\n%s\nFragment shader source:\n%s\n",
         shaderSources.vertexShaderSource, shaderSources.fragmentShaderSource);

  unsigned int shaderProgram = createShader(shaderSources.vertexShaderSource,
                                            shaderSources.fragmentShaderSource);

  // Define the vertices for a triangle
  float vertices[] = {-0.5f, 0.5f,  0.0f, 0.5f,  0.5f,  0.0f,
                      0.5f,  -0.5f, 0.0f, -0.5f, -0.5f, 0.0f};

  unsigned int indices[] = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  // Create and bind a Vertex Array Object
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // Create and bind a Vertex Buffer Object
  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Create and bind a Index Buffer Object
  unsigned int IBO;
  glGenBuffers(1, &IBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // Set the vertex attributes pointers
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Unbind the VBO and VAO and IBO
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // Rendering loop
  while (!glfwWindowShouldClose(window)) {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);

    // Use our shader program
    glUseProgram(shaderProgram);

    // Draw the triangle
    glBindVertexArray(VAO);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }

  // Clean up
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(shaderProgram);

  free(shaderSources.vertexShaderSource);
  free(shaderSources.fragmentShaderSource);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
