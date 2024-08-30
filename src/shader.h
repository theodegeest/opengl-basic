#ifndef SHADER_H_
#define SHADER_H_

#include <stdlib.h>
#include "map.h"

typedef struct {
  char *vertexShaderSource;
  char *fragmentShaderSource;
} ShaderSources;

typedef struct {
  unsigned int id;
  const char *filePath;
  ShaderSources sources;
  MapCharInt *locations;
} Shader;

Shader *shader_create(const char *filePath);
void shader_free(Shader *shader);

void shader_bind(Shader *shader);
void shader_unbind();

void shader_uniform_set_4f(Shader *shader, const char *name, size_t name_size,
                           float v0, float v1, float v2, float v3);

#endif // !SHADER_H_
