#ifndef SHADER_H_
#define SHADER_H_

#include "../map.h"
#include <cglm/types.h>
#include <stdlib.h>

typedef struct {
  char *vertexShaderSource;
  char *fragmentShaderSource;
} ShaderSources;

typedef struct {
  unsigned int id;
  const char *file_path;
  ShaderSources sources;
  MapCharInt *locations;
} Shader;

Shader *shader_create(const char *file_path);
void shader_free(Shader *shader);

void shader_bind(Shader *shader);
void shader_unbind();

void shader_uniform_set_1i(Shader *shader, const char *name, int v0);
void shader_uniform_set_1iv(Shader *shader, const char *name, int count, int *data);
void shader_uniform_set_1f(Shader *shader, const char *name, float v0);
void shader_uniform_set_4f(Shader *shader, const char *name, float v0, float v1,
                           float v2, float v3);
void shader_uniform_set_mat4f(Shader *shader, const char *name, mat4 v0);

#endif // !SHADER_H_
