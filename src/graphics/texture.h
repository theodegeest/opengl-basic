#ifndef TEXTURE_H_
#define TEXTURE_H_

typedef struct {
  unsigned int id;
  const char *file_path;
  unsigned char *local_buffer;
  int width;
  int height;
  int bbp;
} Texture;

Texture *texture_create(const char *file_path);
void texture_free(Texture *texture);

void texture_bind(Texture *texture, unsigned int slot);
void texture_unbind(Texture *texture);

#endif // !TEXTURE_H_
