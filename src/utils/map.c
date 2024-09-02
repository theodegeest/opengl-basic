#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO:
// https://stackoverflow.com/questions/6669842/how-to-best-achieve-string-to-number-mapping-in-a-c-program

static unsigned int _map_increase_size(MapCharInt *map);

/******************************************************************************
                              PUBLIC FUNCTIONS                                *
*******************************************************************************/

MapCharInt *map_char_int_create() {
  MapCharInt *map = malloc(sizeof(MapCharInt));

  map->entries = NULL;
  map->size = 0;
  map->max_capacity = 0;

  return map;
}
void map_char_int_free(MapCharInt *map) {
  free(map->entries);
  free(map);
}

unsigned int map_char_int_put(MapCharInt *map, const char *key, int value) {
  if (map->size == map->max_capacity) {
    if (!_map_increase_size(map)) {
      return 0;
    }
  }

  map->entries[map->size++] = (MapEntryCharInt){key, value};
  return 1;
}

int map_char_int_get(MapCharInt *map, const char *key) {
  for (int i = 0; i < map->size; i++) {
    if (strcmp(map->entries[i].key, key) == 0) {
      return map->entries[i].value;
    }
  }
  return -1;
}

unsigned int map_char_int_remove(MapCharInt *map, const char *key) {
  printf("TODO map remove");
  return 0;
}

/******************************************************************************
                              PRIVATE FUNCTIONS                               *
*******************************************************************************/

static unsigned int _map_increase_size(MapCharInt *map) {
  map->max_capacity *= 2;

  if (map->max_capacity == 0) {
    map->max_capacity = 1;
  }

  map->entries =
      realloc(map->entries, map->max_capacity * sizeof(MapEntryCharInt));

  if (map->entries == NULL) {
    fprintf(stderr, "Failed to allocate memory for map\n");
    return 0;
  }

  return 1;
}
