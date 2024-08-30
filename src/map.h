#ifndef MAP_H_
#define MAP_H_
typedef struct  {
  const char *key;
  int value;
} MapEntryCharInt;

typedef struct {
  MapEntryCharInt *entries;
  unsigned int size;
  unsigned int max_capacity;
} MapCharInt;

MapCharInt *map_char_int_create();
void map_char_int_free(MapCharInt *map);

unsigned int map_char_int_put(MapCharInt *map, const char *key, int value);
int map_char_int_get(MapCharInt *map, const char *key);
unsigned int map_char_int_remove(MapCharInt *map, const char *key);


#endif // !MAP_H_
