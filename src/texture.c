#include "texture.h"
#include <stdio.h>
#include <stdlib.h>

#include "../include/glad/glad.h"
#include "../include/stb/std_image.h"

/******************************************************************************
                              PUBLIC FUNCTIONS                                *
*******************************************************************************/

Texture *texture_create(const char *file_path) {
  Texture *texture = malloc(sizeof(Texture));

  texture->file_path = file_path;
  texture->local_buffer = NULL;
  texture->width = 0;
  texture->height = 0;
  texture->bbp = 0;


  stbi_set_flip_vertically_on_load(1);
  texture->local_buffer =
      stbi_load(file_path, &texture->width, &texture->height, &texture->bbp, 4);

  glGenTextures(1, &texture->id);
  glBindTexture(GL_TEXTURE_2D, texture->id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width, texture->height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, texture->local_buffer);
  glBindTexture(GL_TEXTURE_2D, 0);

  if (texture->local_buffer) {
    stbi_image_free(texture->local_buffer);
    printf("The texture at '%s' has been opened successfully\n", file_path);
  } else {
    printf("The texture at '%s' has NOT been opened successfully\n", file_path);
  }

  return texture;
}
void texture_free(Texture *texture) {
  glDeleteTextures(1, &texture->id);
  free(texture);
}

void texture_bind(Texture *texture, unsigned int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, texture->id);
}
void texture_unbind(Texture *texture) { glBindTexture(GL_TEXTURE_2D, 0); }

/******************************************************************************
                              PRIVATE FUNCTIONS                               *
*******************************************************************************/
