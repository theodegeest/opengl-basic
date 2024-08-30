#include "debug.h"

#include <stdio.h>

#include "../include/glad/glad.h"

void glClearError() {
  while (glGetError() != GL_NO_ERROR) {
  }
}

void glLogError(const char *function_call, const char *file, int line) {
  GLenum error;
  while ((error = glGetError())) {
    printf("[OpenGL Error] (%u): file: %s, line:%d\n", error, file, line);
  }
}
