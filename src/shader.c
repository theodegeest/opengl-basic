#include "shader.h"

#include "../include/glad/glad.h"
#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int _shader_uniform_location_get(Shader *shader, const char *name);
static ShaderSources _shader_load_file(const char *file_path);
static unsigned int _shader_compile(unsigned int type, const char *source);
static unsigned int _shader_create(const char *vertexShaderSource,
                                   const char *fragmentShaderSource);

/******************************************************************************
                              PUBLIC FUNCTIONS                                *
*******************************************************************************/

Shader *shader_create(const char *file_path) {
  Shader *shader = malloc(sizeof(Shader));

  shader->id = 0;
  shader->file_path = file_path;

  ShaderSources sources = _shader_load_file(file_path);
  shader->sources = sources;

  printf("Vertex shader source:\n%s\nFragment shader source:\n%s\n",
         sources.vertexShaderSource, sources.fragmentShaderSource);

  shader->id =
      _shader_create(sources.vertexShaderSource, sources.fragmentShaderSource);

  shader->locations = map_char_int_create();

  return shader;
}

void shader_free(Shader *shader) {
  glDeleteProgram(shader->id);
  free(shader->sources.vertexShaderSource);
  free(shader->sources.fragmentShaderSource);
  map_char_int_free(shader->locations);
  free(shader);
}

void shader_bind(Shader *shader) { glUseProgram(shader->id); }
void shader_unbind() { glUseProgram(0); }

void shader_uniform_set_1i(Shader *shader, const char *name, int v0) {
  glUniform1i(_shader_uniform_location_get(shader, name), v0);
}

void shader_uniform_set_1f(Shader *shader, const char *name, float v0) {
  glUniform1f(_shader_uniform_location_get(shader, name), v0);
}

void shader_uniform_set_4f(Shader *shader, const char *name, float v0, float v1,
                           float v2, float v3) {
  glUniform4f(_shader_uniform_location_get(shader, name), v0, v1, v2, v3);
}

/******************************************************************************
                              PRIVATE FUNCTIONS                               *
*******************************************************************************/

static int _shader_uniform_location_get(Shader *shader, const char *name) {
  int cached_location = map_char_int_get(shader->locations, name);
  if (cached_location != -1) {
    // printf("Location has been found in cache\n");
    return cached_location;
  }

  // printf("Location has not been found in cache\n");
  int location = glGetUniformLocation(shader->id, name);

  if (location == -1) {
    printf("[Shader Warning] Uniform location of '%s' doesn't exist\n", name);
  } else {
    map_char_int_put(shader->locations, name, location);
  }

  return location;
}

static ShaderSources _shader_load_file(const char *file_path) {
  ShaderSources sources = {NULL, NULL};
  FILE *file = fopen(file_path, "r");

  if (!file) {
    fprintf(stderr, "Failed to open shader file: %s\n", file_path);
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
static unsigned int _shader_compile(unsigned int type, const char *source) {
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

static unsigned int _shader_create(const char *vertexShaderSource,
                                   const char *fragmentShaderSource) {
  // Build and compile our shader program
  unsigned int vertexShader =
      _shader_compile(GL_VERTEX_SHADER, vertexShaderSource);
  unsigned int fragmentShader =
      _shader_compile(GL_FRAGMENT_SHADER, fragmentShaderSource);

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
