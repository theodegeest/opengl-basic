#include "../../include/glad/glad.h"

#include "../graphics/graphics.h"
#include "../graphics/index_buffer.h"
#include "../graphics/renderer.h"
#include "../graphics/shader.h"
#include "../graphics/texture.h"
#include "../graphics/vertex_array.h"
#include "../mesh/shape.h"
#include "../third-party/c-thread-pool/thpool.h"
#include "../utils/timer.h"
#include "scene_pixel_sim.h"
#include <cglm/cglm.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../include/Nuklear/nuklear.h"

#define SIM_WIDTH (25 * 25)
#define SIM_HEIGHT (25 * 18)
#define SIM_CELL_SIZE 2

#define SIM_CELL_COUNT (SIM_WIDTH * SIM_HEIGHT)

#define SIM_CHUNK_SIZE 25
#define SIM_CHUNK_WIDTH (SIM_WIDTH / SIM_CHUNK_SIZE)
#define SIM_CHUNK_HEIGHT (SIM_HEIGHT / SIM_CHUNK_SIZE)
#define SIM_CHUNK_SIZE_PIXEL (SIM_CELL_SIZE * SIM_CHUNK_SIZE)
#define SIM_CHUNK_CELL_COUNT (SIM_CHUNK_SIZE * SIM_CHUNK_SIZE)
#define SIM_CHUNK_COUNT (SIM_CELL_COUNT / SIM_CHUNK_CELL_COUNT)

#define SIM_WIDTH_PIXEL (SIM_WIDTH * SIM_CELL_SIZE)
#define SIM_HEIGHT_PIXEL (SIM_HEIGHT * SIM_CELL_SIZE)

#define HORIZONTAL_OFFSET ((WINDOW_WIDTH / 2.0f) - (SIM_WIDTH_PIXEL / 2.0f))
#define VERTICAL_OFFSET 50.0f

#define NUM_THREADS 8

typedef struct {
  int dirty_bit;
  int had_changed;
  Quad *first_element;
  int min_x, max_x, min_y, max_y;
} Chunk;

typedef struct {
  VertexArray *va_cells;
  VertexBuffer *vb_cells;
  IndexBuffer *ib_cells;
  VertexBufferLayout *layout_cells;
  VertexArray *va_chunks;
  VertexBuffer *vb_chunks;
  IndexBuffer *ib_chunks;
  VertexBufferLayout *layout_chunks;
  VertexArray *va_chunks_dirty_rect;
  VertexBuffer *vb_chunks_dirty_rect;
  IndexBuffer *ib_chunks_dirty_rect;
  VertexBufferLayout *layout_chunks_dirty_rect;
  Shader *shader_cells;
  Shader *shader_chunks;
  Shader *shader_chunks_dirty_rect;
  Texture *texture_air;
  Texture *texture_water;
  Texture *texture_sand;
  Renderer *renderer;
  float value_x;
  float value_y;
  mat4 proj;
  mat4 view;
  char show_chunks;
  char show_chunks_clear;
  Chunk *chunks;
  threadpool threadpool;
  struct sim_args *args;
} PixelSimObj;

typedef enum {
  TYPE_AIR,
  TYPE_WATER,
  TYPE_SAND,
  TYPE_OUTOFBOUNDS,
  TYPE_COUNT
} SimType;

typedef enum {
  ACTION_NONE,
  ACTION_SWAP_DOWN,
  ACTION_SWAP_DOWN_LEFT,
  ACTION_SWAP_DOWN_RIGHT,
  ACTION_GOTO_NEXT
} UpdateAction;

typedef enum {
  DIR_DOWN,
  DIR_LEFT_DOWN,
  DIR_RIGHT_DOWN,
  DIR_ANY,
  DIR_COUNT
} Direction;

typedef enum {
  INSTRUCTION_SEQ,
  INSTRUCTION_RND,
  INSTRUCTION_LEAF
} InstructionType;

// typedef struct {
// } InstructionHeader;

// typedef struct {
//   InstructionHeader header;
// } Instruction;

typedef struct {
  InstructionType type;
  unsigned int count;
  struct InstructionNode *children;
  Direction dir;
  UpdateAction action[TYPE_COUNT];
} InstructionNode;

// TODO: Maybe optimize the next two functions
static Chunk *chunk_get(PixelSimObj *obj, int x, int y) {
  // if (x < 0 || x >= SIM_WIDTH || y < 0 || y >= SIM_HEIGHT) {
  //   return NULL;
  // }
  const int chunk_id_x = x / SIM_CHUNK_SIZE;
  const int chunk_id_y = y / SIM_CHUNK_SIZE;
  // int cell_id = (y * SIM_WIDTH) + x;
  // int chunk_id = cell_id / SIM_CHUNK_CELL_COUNT;
  const int chunk_id = (chunk_id_y * SIM_CHUNK_WIDTH) + chunk_id_x;
  // printf("x: %d, y: %d, chunk: %d\n", x, y, chunk_id);
  return &obj->chunks[chunk_id];
}

static Quad *cell_get(PixelSimObj *obj, int x, int y) {
  if (x < 0 || x >= SIM_WIDTH || y < 0 || y >= SIM_HEIGHT) {
    return NULL;
  }
  const Chunk *chunk = chunk_get(obj, x, y);
  const int chunk_x = x % SIM_CHUNK_SIZE;
  const int chunk_y = y % SIM_CHUNK_SIZE;
  const int chunk_index = (chunk_y * SIM_CHUNK_SIZE) + chunk_x;
  // printf("%d, %d, %d\n", chunk_x, chunk_y, chunk_index);
  return &chunk->first_element[chunk_index];
}

// typedef struct {
//   int number_of_instuctions;
//   InstructionNode *instruction_node;
// } InstructionInfo;

// on instructions[TYPE] are the instructions for that type.
InstructionNode *instructions;
// InstructionInfo instructions[TYPE_COUNT] = {
//     // AIR
//     {1,
//      (Instruction[]){
//          {DIR_ANY, {ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE}}}},
//
//     // WATER
//     {1,
//      (Instruction[]){
//          {DIR_ANY, {ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE}}}},
//
//     // SAND
//     {2, (Instruction*)&({(Instruction){DIR_DOWN,
//                                       {ACTION_SWAP_DOWN, ACTION_SWAP_DOWN,
//                                        ACTION_GOTO_NEXT, ACTION_NONE}},
//                         (Instruction){DIR_LEFT_DOWN,
//                                       {ACTION_SWAP_DOWN, ACTION_SWAP_DOWN,
//                                        ACTION_GOTO_NEXT, ACTION_NONE}}}},
//
//     // OUTOFBOUNDS
//     {1, (Instruction[]){
//             {DIR_ANY, {ACTION_NONE, ACTION_NONE, ACTION_NONE,
//             ACTION_NONE}}}}};

// typedef struct {
//   SimType current;
//   SimType down;
//   SimType down_left;
//   SimType down_right;
// } SimNeighborhood;
//
// uint32_t hash_pixel_neighborhood(const SimNeighborhood *key) {
//   uint32_t hash = key->current;
//   hash = hash * 31 + key->down;
//   hash = hash * 31 + key->down_left;
//   hash = hash * 31 + key->down_right;
//   return hash;
// }
//
// int equals_pixel_neighborhood(const SimNeighborhood *a,
//                               const SimNeighborhood *b) {
//   return (a->current == b->current) && (a->down == b->down) &&
//          (a->down_left == b->down_left) && (a->down_right == b->down_right);
// }
//
// typedef struct HashmapEntry {
//   SimNeighborhood key;
//   UpdateAction action;
//   struct HashmapEntry *next;
// } HashmapEntry;
//
// #define HASHMAP_SIZE 1024 // Example size; adjust as needed
//
// HashmapEntry *hashmap[HASHMAP_SIZE];
//
// void insert_rule(SimNeighborhood key, UpdateAction action) {
//   uint32_t index = hash_pixel_neighborhood(&key) % HASHMAP_SIZE;
//
//   HashmapEntry *new_entry = malloc(sizeof(HashmapEntry));
//   new_entry->key = key;
//   new_entry->action = action;
//   new_entry->next = hashmap[index];
//
//   hashmap[index] = new_entry;
// }
//
// UpdateAction find_action(SimNeighborhood key) {
//   uint32_t index = hash_pixel_neighborhood(&key) % HASHMAP_SIZE;
//
//   HashmapEntry *entry = hashmap[index];
//   while (entry != NULL) {
//     if (equals_pixel_neighborhood(&entry->key, &key)) {
//       return entry->action;
//     }
//     entry = entry->next;
//   }
//
//   return ACTION_NONE; // Default action if no match is found
// }
static char check_boundaries(int x, int y) {
  return x >= 0 && x < SIM_WIDTH && y >= 0 && y < SIM_HEIGHT;
}

static SimType get_type_from_texture_id(float texture_id) {
  return (SimType)texture_id;
}

static float get_texture_id(SimType type) { return (float)type; }

static SimType get_type(int x, int y, Quad *q) {
  if (x < 0 || x >= SIM_WIDTH || y < 0 || y >= SIM_HEIGHT) {
    return TYPE_OUTOFBOUNDS;
  }
  // int i = SIM_WIDTH * y + x;
  // Quad *q = vertex_buffer_quad_get(vb, i * 4);
  return get_type_from_texture_id(quad_texture_id_get(q));
}

// static SimType get_type_from_dir(VertexBuffer *vb, int x, int y,
//                                  Direction dir) {
//   switch (dir) {
//   case DIR_DOWN:
//     return get_type(vb, x, y - 1);
//   case DIR_LEFT_DOWN:
//     return get_type(vb, x - 1, y - 1);
//   case DIR_RIGHT_DOWN:
//     return get_type(vb, x + 1, y - 1);
//   case DIR_ANY:
//     printf("Should not receive DIR_ANY in get_type_from_dir\n");
//     return TYPE_OUTOFBOUNDS;
//   case DIR_COUNT:
//     printf("Should not receive DIR_ANY in get_type_from_dir\n");
//     return TYPE_OUTOFBOUNDS;
//   }
//
//   return TYPE_OUTOFBOUNDS;
// }

// static SimNeighborhood get_neighborhood(VertexBuffer *vb, int x, int y) {
//   return (SimNeighborhood){get_type(vb, x, y), get_type(vb, x, y - 1),
//                            get_type(vb, x - 1, y - 1),
//                            get_type(vb, x + 1, y - 1)};
// }

static void set_type(int x, int y, Quad *q, SimType type) {
  // if (x < 0 || x >= SIM_WIDTH || y < 0 || y >= SIM_HEIGHT) {
  //   printf("set_type out of bounds\n");
  //   return;
  // }
  // int i = SIM_WIDTH * y + x;
  // Quad *q = vertex_buffer_quad_get(vb, i * 4);
  quad_texture_id_set(q, type);
}

void keep_active(Chunk *chunk, int x, int y) {
  if (chunk->min_x > x) {
    chunk->min_x = x;
  }
  if (chunk->max_x <= x) {
    chunk->max_x = x + 1;
  }
  if (chunk->min_y > y) {
    chunk->min_y = y;
  }
  if (chunk->max_y <= y) {
    chunk->max_y = y + 1;
  }
}

static void set_dirty(PixelSimObj *obj, int x, int y) {
  Chunk *chunk = chunk_get(obj, x, y);
  chunk->dirty_bit = 1;
  keep_active(chunk, x, y);
}

static void set_dirty_safe(PixelSimObj *obj, int x, int y) {
  if (check_boundaries(x, y)) {
    set_dirty(obj, x, y);
  }
}

// static void set_dirty_neighbors(PixelSimObj *obj, int x, int y) {
//   for (int local_y = y - 1; local_y <= y + 1; local_y++) {
//     if (local_y < 0 || local_y >= SIM_HEIGHT) {
//       continue;
//     }
//
//     for (int local_x = x - 1; local_x <= x + 1; local_x++) {
//       if (local_x < 0 || local_x >= SIM_WIDTH) {
//         continue;
//       }
//
//       set_dirty(obj, local_x, local_y);
//       // printf("in %d, %d\n", local_x, local_y);
//     }
//   }
// }

static void set_type_and_dirty(PixelSimObj *obj, int x, int y, Quad *q,
                               SimType type) {
  set_type(x, y, q, type);
  set_dirty(obj, x, y);
}

// Returns 0 when no changes, returns 1 when something has changed
static unsigned char update_sim_sand(PixelSimObj *obj, int x, int y, Quad *q1) {
  Quad *q2 = cell_get(obj, x, y - 1);
  unsigned char changed = 0;
  // Check under
  switch (get_type(x, y - 1, q2)) {
  case TYPE_AIR:
    set_type(x, y, q1, TYPE_AIR);
    set_type_and_dirty(obj, x, y - 1, q2, TYPE_SAND);
    changed = 1;
    break;
  case TYPE_WATER:
    set_type(x, y, q1, TYPE_WATER);
    set_type_and_dirty(obj, x, y - 1, q2, TYPE_SAND);
    changed = 1;
    break;
  case TYPE_SAND:
    // Check left
    q2 = cell_get(obj, x - 1, y - 1);
    switch (get_type(x - 1, y - 1, q2)) {
    case TYPE_AIR:
      set_type(x, y, q1, TYPE_AIR);
      set_type_and_dirty(obj, x - 1, y - 1, q2, TYPE_SAND);
      changed = 1;
      break;
    case TYPE_WATER:
      set_type(x, y, q1, TYPE_WATER);
      set_type_and_dirty(obj, x - 1, y - 1, q2, TYPE_SAND);
      changed = 1;
      break;
    case TYPE_SAND:
      // Check right
      q2 = cell_get(obj, x + 1, y - 1);
      switch (get_type(x + 1, y - 1, q2)) {
      case TYPE_AIR:
        set_type(x, y, q1, TYPE_AIR);
        set_type_and_dirty(obj, x + 1, y - 1, q2, TYPE_SAND);
        changed = 1;
        break;
      case TYPE_WATER:
        set_type(x, y, q1, TYPE_WATER);
        set_type_and_dirty(obj, x + 1, y - 1, q2, TYPE_SAND);
        changed = 1;
        break;
      case TYPE_SAND:
        break;
      case TYPE_OUTOFBOUNDS:
      case TYPE_COUNT:
        break;
      }
      break;
    case TYPE_OUTOFBOUNDS:
    case TYPE_COUNT:
      break;
    }
    break;
  case TYPE_OUTOFBOUNDS:
  case TYPE_COUNT:
    break;
  }

  if (changed) {
    set_dirty_safe(obj, x - 1, y - 1);
    set_dirty_safe(obj, x - 1, y + 1);
    set_dirty_safe(obj, x + 1, y + 1);
    set_dirty_safe(obj, x + 1, y - 1);
    // unsigned int chunk_x = x % SIM_CHUNK_SIZE;
    // unsigned int chunk_y = y % SIM_CHUNK_SIZE;
    //
    // if (chunk_x == 0 && x != 0) {
    //   // left
    //   set_dirty(obj, x - 1, y);
    //   if (chunk_y == 0 && y != 0) {
    //     // left and down
    //     set_dirty(obj, x, y - 1);
    //     set_dirty(obj, x - 1, y - 1);
    //   } else if (chunk_y == SIM_CHUNK_SIZE - 1 && y != SIM_HEIGHT - 1) {
    //     // left and up
    //     set_dirty(obj, x, y + 1);
    //     set_dirty(obj, x - 1, y + 1);
    //   }
    // } else if (chunk_x == SIM_CHUNK_SIZE - 1 && x != SIM_WIDTH - 1) {
    //   // right
    //   set_dirty(obj, x + 1, y);
    //   if (chunk_y == 0 && y != 0) {
    //     // right and down
    //     set_dirty(obj, x, y - 1);
    //     set_dirty(obj, x + 1, y - 1);
    //   } else if (chunk_y == SIM_CHUNK_SIZE - 1 && y != SIM_HEIGHT - 1) {
    //     // right and up
    //     set_dirty(obj, x, y + 1);
    //     set_dirty(obj, x + 1, y + 1);
    //   }
    // } else if (chunk_y == 0 && y != 0) {
    //   // down
    //   set_dirty(obj, x, y - 1);
    // } else if (chunk_y == SIM_CHUNK_SIZE - 1 && y != SIM_HEIGHT - 1) {
    //   // up
    //   set_dirty(obj, x, y + 1);
    // }

    // if (chunk_x == 0 || chunk_x == SIM_CHUNK_SIZE - 1 || chunk_y == 0 ||
    //     chunk_y == SIM_CHUNK_SIZE - 1) {
    //   // printf("edge %d, %d\n", chunk_x, chunk_y);
    //   set_dirty_neighbors(obj, x, y);
    // }
  }
  return changed;
}

// Returns 0 when no changes, returns 1 when something has changed
static unsigned char update_sim(PixelSimObj *obj, int x, int y) {
  // UpdateAction action = find_action(get_neighborhood(vb, x, y));
  // printf("%d\n", action);

  // printf("debug: %d\n", instructions[TYPE_AIR].instruction[0].dir);
  // SimType type = get_type(vb, x, y);
  // UpdateAction action = ACTION_GOTO_NEXT;
  // SimType neighbor_type;
  // InstructionNode node = instructions[type];
  // // while (action == ACTION_GOTO_NEXT) {
  // InstructionType instruction_type = node.type;
  //
  // switch (instruction_type) {
  // case INSTRUCTION_SEQ:
  //   int try = 0;
  //   while (action == ACTION_GOTO_NEXT) {
  //     InstructionNode child = ((InstructionNode *)node.children)[try];
  //     Direction dir = child.dir;
  //     // printf("%d\n", type);
  //     if (dir == DIR_ANY) {
  //       action = child.action[0];
  //     } else {
  //       neighbor_type = get_type_from_dir(vb, x, y, dir);
  //       action = child.action[neighbor_type];
  //     }
  //
  //     try++;
  //   }
  //   break;
  //
  // case INSTRUCTION_RND:
  //   break;
  //
  // case INSTRUCTION_LEAF:
  //   break;
  // }
  // // }

  // switch (action) {
  // case ACTION_NONE:
  //   break;
  // case ACTION_SWAP_DOWN:
  //   set_type(vb, x, y, neighbor_type);
  //   set_type(vb, x, y - 1, type);
  //   break;
  // case ACTION_SWAP_DOWN_LEFT:
  //   set_type(vb, x, y, neighbor_type);
  //   set_type(vb, x - 1, y - 1, type);
  //   break;
  // case ACTION_SWAP_DOWN_RIGHT:
  //   set_type(vb, x, y, neighbor_type);
  //   set_type(vb, x + 1, y - 1, type);
  //   break;
  // case ACTION_GOTO_NEXT:
  //   printf("Something has gone wrong in update_sim because I see a "
  //          "ACTION_GOTO_NEXT\n");
  //   break;
  // }

  unsigned char changed = 0;
  Quad *q = cell_get(obj, x, y);
  if (q == NULL) {
    printf("DEBUG, %d, %d\n", x, y);
  }
  SimType type = get_type(x, y, q);
  switch (type) {
  case TYPE_AIR:
    break;
  case TYPE_WATER:
    break;
  case TYPE_SAND:
    changed = update_sim_sand(obj, x, y, q);
    break;
  case TYPE_OUTOFBOUNDS:
  case TYPE_COUNT:
    break;
  }

  return changed;
}

struct sim_args {
  unsigned int chunk_x;
  unsigned int chunk_y;
  unsigned int chunk_id;
  PixelSimObj *p_obj;
};

void sim_task(void *arg) {
  struct sim_args *args = (struct sim_args *)arg;
  const unsigned int chunk_id = args->chunk_id;
  PixelSimObj *p_obj = args->p_obj;

  Chunk *const chunk = &p_obj->chunks[chunk_id];

  chunk->had_changed = chunk->dirty_bit;

  if (chunk->dirty_bit) {
    // printf("Thread #%u working\n", (int)pthread_self());
    const unsigned int chunk_x = args->chunk_x;
    const unsigned int chunk_y = args->chunk_y;
    const int min_x = chunk->min_x;
    const int max_x = chunk->max_x;
    const int min_y = chunk->min_y;
    const int max_y = chunk->max_y;

    chunk->min_x = (chunk_x + 1) * SIM_CHUNK_SIZE;
    chunk->max_x = chunk_x * SIM_CHUNK_SIZE;
    chunk->min_y = (chunk_y + 1) * SIM_CHUNK_SIZE;
    chunk->max_y = chunk_y * SIM_CHUNK_SIZE;

    ShapeBox *const chunk_box = (ShapeBox *)vertex_buffer_get(
        p_obj->vb_chunks, chunk_id * SHAPE_BOX_NUMBER_OF_VERTICES,
        SHAPE_BOX_NUMBER_OF_VERTICES);

    ShapeBox *const chunk_dirty_rect_box = (ShapeBox *)vertex_buffer_get(
        p_obj->vb_chunks_dirty_rect, chunk_id * SHAPE_BOX_NUMBER_OF_VERTICES,
        SHAPE_BOX_NUMBER_OF_VERTICES);
    int chunk_changed = 0;
    // for (unsigned int local_y = 0; local_y < SIM_CHUNK_SIZE; local_y++)
    // {
    //   unsigned int y = (chunk_id_y * SIM_CHUNK_SIZE) + local_y;
    //   for (unsigned int local_x = 0; local_x < SIM_CHUNK_SIZE;
    //   local_x++)
    //   {
    //     unsigned int x = (chunk_id_x * SIM_CHUNK_SIZE) + local_x;
    //     unsigned char changed = update_sim(p_obj, x, y);
    //     if (changed) {
    //       // printf("Changed -> x: %d, y: %d\n", x, y);
    //       chunk_changed = 1;
    //       keep_active(chunk, x, y);
    //     }
    //   }
    // }
    for (unsigned int y = min_y; y < max_y; y++) {
      for (unsigned int x = min_x; x < max_x; x++) {
        unsigned char changed = update_sim(p_obj, x, y);
        if (changed) {
          // printf("Changed -> x: %d, y: %d\n", x, y);
          chunk_changed = 1;
          keep_active(chunk, x, y);
        }
      }
    }
    // printf("min_x: %d, min_y: %d, max_x: %d, max_y: %d\n",
    // chunk->min_x,
    //        chunk->min_y, chunk->max_x, chunk->max_y);

    // for (unsigned int y = chunk_id_y * SIM_CHUNK_SIZE;
    //      y < (chunk_id_y + 1) * SIM_CHUNK_SIZE; y++) {
    //   for (unsigned int x = chunk_id_x * SIM_CHUNK_SIZE;
    //        x < (chunk_id_x + 1) * SIM_CHUNK_SIZE; x++) {
    //     unsigned char changed = update_sim(p_obj, x, y);
    //     if (changed) {
    //       // printf("Changed -> x: %d, y: %d\n", x, y);
    //       chunk_changed = 1;
    //       keep_active(chunk, x, y);
    //     }
    //   }
    // }
    chunk->dirty_bit = chunk_changed;
    if (chunk_changed) {
      shape_box_color_set(chunk_box, (Color){1, 0, 0, 1});

      shape_box_color_set(chunk_dirty_rect_box, (Color){0, 0, 0, 1});
      shape_box_move(chunk_dirty_rect_box,
                     HORIZONTAL_OFFSET + chunk->min_x * SIM_CELL_SIZE,
                     VERTICAL_OFFSET + chunk->min_y * SIM_CELL_SIZE,
                     (chunk->max_x - chunk->min_x) * SIM_CELL_SIZE,
                     (chunk->max_y - chunk->min_y) * SIM_CELL_SIZE);
      // printf("Box red\n");
    } else {
      shape_box_color_set(chunk_box, (Color){0, 0, 0, 0});
      shape_box_color_set(chunk_dirty_rect_box, (Color){0, 0, 0, 0});
      // printf("Box black\n");
    }
  }
}

static void on_update(void *obj, float delta_time, GLFWwindow *window) {
  PixelSimObj *p_obj = (PixelSimObj *)obj;

  // puts("Adding 40 tasks to threadpool");
  // int i;
  // for (i = 0; i < 40; i++) {
  //   thpool_add_work(p_obj->threadpool, task, (void *)(uintptr_t)i);
  // };
  //
  // thpool_wait(p_obj->threadpool);
  //
  // printf("After wait\n\n\n");
  // Create Quads
  // vertex_buffer_clear(p_obj->vb);
  //
  // int size = 10;
  //
  // for (int i = 0; i < 70; i++) {
  //   for (int j = 0; j < 50; j++) {
  //     Quad q = quad_create(50.0f + i * size, 50.0f + j * size, 0.0f, size,
  //     size,
  //                          0.0f, (Color){0.0f, 0.0f, 0.0f, 1.0f}, 0);
  //     vertex_buffer_push_quad(p_obj->vb, q);
  //   }
  // }

  vertex_array_bind(p_obj->va_cells);
  // Quad *spawn_q = cell_get(p_obj, SIM_WIDTH / 2, SIM_HEIGHT - 1);
  // quad_texture_id_set(spawn_q, 2);

  Timer *total = timer_init();
  Timer *update_loop = timer_init();
  Timer *mouse_action = timer_init();
  Timer *data_flushing = timer_init();

  timer_start(total);
  timer_start(update_loop);

  for (char phase = 0; phase < 4; phase++) {
    // printf("\n ---------------------------------\nPhase %d\n", phase);
    for (int chunk_y = (phase / 2) % 2; chunk_y < SIM_CHUNK_HEIGHT;
         chunk_y += 2) {
      for (int chunk_x = phase % 2; chunk_x < SIM_CHUNK_WIDTH; chunk_x += 2) {
        const unsigned int chunk_id = (chunk_y * SIM_CHUNK_WIDTH) + chunk_x;

        char bit = p_obj->chunks[chunk_id].dirty_bit ||
                   p_obj->chunks[chunk_id].had_changed;
        if (bit) {
          // sim_task(&p_obj->args[chunk_id]);
          thpool_add_work(p_obj->threadpool, sim_task, &p_obj->args[chunk_id]);
        }
        // }
      }
    }

    // printf("%d\n", thpool_num_threads_working(p_obj->threadpool));

    thpool_wait(p_obj->threadpool);
    // printf("Done with phase %d\n", phase);
  }

  // for (unsigned int chunk_id_y = 0; chunk_id_y < SIM_CHUNK_HEIGHT;
  //      chunk_id_y++) {
  //   for (unsigned int chunk_id_x = 0; chunk_id_x < SIM_CHUNK_WIDTH;
  //        chunk_id_x++) {
  //     const unsigned int chunk_id = (chunk_id_y * SIM_CHUNK_WIDTH) +
  //     chunk_id_x; Chunk *const chunk = &p_obj->chunks[chunk_id];
  //
  //     chunk->had_changed = chunk->dirty_bit;
  //
  //     if (chunk->dirty_bit) {
  //       const int min_x = chunk->min_x;
  //       const int max_x = chunk->max_x;
  //       const int min_y = chunk->min_y;
  //       const int max_y = chunk->max_y;
  //
  //       chunk->min_x = (chunk_id_x + 1) * SIM_CHUNK_SIZE;
  //       chunk->max_x = chunk_id_x * SIM_CHUNK_SIZE;
  //       chunk->min_y = (chunk_id_y + 1) * SIM_CHUNK_SIZE;
  //       chunk->max_y = chunk_id_y * SIM_CHUNK_SIZE;
  //
  //       ShapeBox *const chunk_box = (ShapeBox *)vertex_buffer_get(
  //           p_obj->vb_chunks, chunk_id * SHAPE_BOX_NUMBER_OF_VERTICES,
  //           SHAPE_BOX_NUMBER_OF_VERTICES);
  //
  //       ShapeBox *const chunk_dirty_rect_box = (ShapeBox *)vertex_buffer_get(
  //           p_obj->vb_chunks_dirty_rect,
  //           chunk_id * SHAPE_BOX_NUMBER_OF_VERTICES,
  //           SHAPE_BOX_NUMBER_OF_VERTICES);
  //       int chunk_changed = 0;
  //       // for (unsigned int local_y = 0; local_y < SIM_CHUNK_SIZE;
  //       local_y++) {
  //       //   unsigned int y = (chunk_id_y * SIM_CHUNK_SIZE) + local_y;
  //       //   for (unsigned int local_x = 0; local_x < SIM_CHUNK_SIZE;
  //       local_x++)
  //       //   {
  //       //     unsigned int x = (chunk_id_x * SIM_CHUNK_SIZE) + local_x;
  //       //     unsigned char changed = update_sim(p_obj, x, y);
  //       //     if (changed) {
  //       //       // printf("Changed -> x: %d, y: %d\n", x, y);
  //       //       chunk_changed = 1;
  //       //       keep_active(chunk, x, y);
  //       //     }
  //       //   }
  //       // }
  //       for (unsigned int y = min_y; y < max_y; y++) {
  //         for (unsigned int x = min_x; x < max_x; x++) {
  //           unsigned char changed = update_sim(p_obj, x, y);
  //           if (changed) {
  //             // printf("Changed -> x: %d, y: %d\n", x, y);
  //             chunk_changed = 1;
  //             keep_active(chunk, x, y);
  //           }
  //         }
  //       }
  //       // printf("min_x: %d, min_y: %d, max_x: %d, max_y: %d\n",
  //       chunk->min_x,
  //       //        chunk->min_y, chunk->max_x, chunk->max_y);
  //
  //       // for (unsigned int y = chunk_id_y * SIM_CHUNK_SIZE;
  //       //      y < (chunk_id_y + 1) * SIM_CHUNK_SIZE; y++) {
  //       //   for (unsigned int x = chunk_id_x * SIM_CHUNK_SIZE;
  //       //        x < (chunk_id_x + 1) * SIM_CHUNK_SIZE; x++) {
  //       //     unsigned char changed = update_sim(p_obj, x, y);
  //       //     if (changed) {
  //       //       // printf("Changed -> x: %d, y: %d\n", x, y);
  //       //       chunk_changed = 1;
  //       //       keep_active(chunk, x, y);
  //       //     }
  //       //   }
  //       // }
  //       chunk->dirty_bit = chunk_changed;
  //       if (chunk_changed) {
  //         shape_box_color_set(chunk_box, (Color){1, 0, 0, 1});
  //
  //         shape_box_color_set(chunk_dirty_rect_box, (Color){0, 0, 0, 1});
  //         shape_box_move(chunk_dirty_rect_box,
  //                        HORIZONTAL_OFFSET + chunk->min_x * SIM_CELL_SIZE,
  //                        VERTICAL_OFFSET + chunk->min_y * SIM_CELL_SIZE,
  //                        (chunk->max_x - chunk->min_x) * SIM_CELL_SIZE,
  //                        (chunk->max_y - chunk->min_y) * SIM_CELL_SIZE);
  //         // printf("Box red\n");
  //       } else {
  //         shape_box_color_set(chunk_box, (Color){0, 0, 0, 0});
  //         shape_box_color_set(chunk_dirty_rect_box, (Color){0, 0, 0, 0});
  //         // printf("Box black\n");
  //       }
  //     }
  //   }
  // }

  timer_stop(update_loop);
  timer_start(mouse_action);

  double xpos, ypos;
  char sand_click = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
  char water_click = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
  if (sand_click || water_click) {
    glfwGetCursorPos(window, &xpos, &ypos);

    const double sim_x = xpos - HORIZONTAL_OFFSET;
    const double sim_y = (WINDOW_HEIGHT - ypos) - VERTICAL_OFFSET;

    if (sim_x >= 0 && sim_x < SIM_WIDTH_PIXEL && sim_y >= 0 &&
        sim_y < SIM_HEIGHT_PIXEL) {
      int center_x = ((int)sim_x) / SIM_CELL_SIZE;
      int center_y = ((int)sim_y) / SIM_CELL_SIZE;
      // printf("%u, %u\n", center_x, center_y);

      SimType type;
      if (sand_click) {
        type = TYPE_SAND;
      } else if (water_click) {
        type = TYPE_WATER;
      } else {
        printf("Error: click of unknown type\n");
      }

      const int effect_radius = 5;
      for (int y = center_y - effect_radius; y < center_y + effect_radius;
           y++) {
        if (y < 0 || y >= SIM_HEIGHT) {
          continue;
        }
        for (int x = center_x - effect_radius; x < center_x + effect_radius;
             x++) {
          if (x < 0 || x >= SIM_WIDTH) {
            continue;
          }

          Quad *q = cell_get(p_obj, x, y);
          set_type_and_dirty(p_obj, x, y, q, type);
        }
      }
    }
  }

  timer_stop(mouse_action);
  timer_start(data_flushing);

  for (unsigned int chunk_id_y = 0; chunk_id_y < SIM_CHUNK_HEIGHT;
       chunk_id_y++) {
    for (unsigned int chunk_id_x = 0; chunk_id_x < SIM_CHUNK_WIDTH;
         chunk_id_x++) {
      const unsigned int chunk_id = (chunk_id_y * SIM_CHUNK_WIDTH) + chunk_id_x;
      Chunk *chunk = &p_obj->chunks[chunk_id];
      if (chunk->had_changed) {
        const unsigned int index =
            chunk_id * SIM_CHUNK_CELL_COUNT * QUAD_NUMBER_OF_VERTICES;
        const unsigned int number_of_vertices =
            SIM_CHUNK_CELL_COUNT * QUAD_NUMBER_OF_VERTICES;

        // printf("chunk_id: %d, index: %d, number_of_vertices: %d\n", chunk_id,
        //        index, number_of_vertices);
        vertex_buffer_flush_part(p_obj->vb_cells, index, number_of_vertices);

        if (p_obj->show_chunks) {
          const unsigned int chunk_index =
              chunk_id * SHAPE_BOX_NUMBER_OF_VERTICES;
          const unsigned int chunk_number_of_vertices =
              SHAPE_BOX_NUMBER_OF_VERTICES;
          vertex_buffer_flush_part(p_obj->vb_chunks, chunk_index,
                                   chunk_number_of_vertices);

          vertex_buffer_flush_part(p_obj->vb_chunks_dirty_rect, chunk_index,
                                   chunk_number_of_vertices);
        }
      }
    }
  }

  if (p_obj->show_chunks_clear) {
    p_obj->show_chunks_clear = 0;
    vertex_buffer_flush(p_obj->vb_chunks);
    vertex_buffer_flush(p_obj->vb_chunks_dirty_rect);
  }

  timer_stop(data_flushing);
  timer_stop(total);

  printf("Total: %lf, update: %lf, mouse: %lf, data: %lf\n",
         timer_elapsed(total), timer_elapsed(update_loop),
         timer_elapsed(mouse_action), timer_elapsed(data_flushing));

  // for (int i = 0; i < total; i++) {
  //   int x = i % SIM_WIDTH;
  //   int y = i / SIM_WIDTH;
  //
  //   // Quad *current = vertex_buffer_quad_get(p_obj->vb, i);
  //   // if (current == NULL) {
  //   //   printf("Could not get quad\n");
  //   // }
  //
  //   unsigned char changed = update_sim(p_obj, x, y);
  //   if (changed) {
  //     printf("Changed -> x: %d, y: %d\n", x, y);
  //   }
  //   // quad_texture_id_set(current, 1.0f);
  // }

  // vertex_buffer_flush(p_obj->vb_cells);
  // vertex_buffer_flush(p_obj->vb_chunks);
  // vertex_buffer_flush(p_obj->vb_chunks_dirty_rect);
}

static void on_render(void *obj) {
  // printf("Clear Color On Render\n");
  PixelSimObj *p_obj = (PixelSimObj *)obj;
  renderer_clear(p_obj->renderer);

  // Use our shader program
  shader_bind(p_obj->shader_cells);

  texture_bind(p_obj->texture_air, 0);
  texture_bind(p_obj->texture_water, 1);
  texture_bind(p_obj->texture_sand, 2);

  {
    mat4 model;
    glm_translate_make(model, (vec3){0.0f, 0.0f, 0.0f});
    glm_translate(model, (vec3){p_obj->value_x, p_obj->value_y, 0.0f});

    mat4 mvp;
    glm_mat4_mul(p_obj->proj, p_obj->view, mvp);
    glm_mat4_mul(mvp, model, mvp);

    shader_uniform_set_mat4f(p_obj->shader_cells, "u_MVP", mvp);

    renderer_draw(p_obj->renderer, p_obj->va_cells, p_obj->vb_cells,
                  p_obj->shader_cells);
  }

  // {
  //   mat4 model;
  //   glm_translate_make(model, (vec3){300.0f, 100.0f, 0.0f});
  //   glm_translate(
  //       model, (vec3){c_obj->value_x * 2, c_obj->value_y * 2,
  //       c_obj->value_z});
  //
  //   mat4 mvp;
  //   glm_mat4_mul(c_obj->proj, c_obj->view, mvp);
  //   glm_mat4_mul(mvp, model, mvp);
  //
  //   shader_uniform_set_mat4f(c_obj->shader, "u_MVP", mvp);
  //
  //   renderer_draw(c_obj->renderer, c_obj->va, c_obj->ib, c_obj->shader);
  // }

  vertex_array_unbind();

  if (p_obj->show_chunks) {
    // Draw chunks
    shader_bind(p_obj->shader_chunks);

    {
      mat4 model;
      glm_translate_make(model, (vec3){0.0f, 0.0f, 0.0f});
      glm_translate(model, (vec3){p_obj->value_x, p_obj->value_y, 0.0f});

      mat4 mvp;
      glm_mat4_mul(p_obj->proj, p_obj->view, mvp);
      glm_mat4_mul(mvp, model, mvp);

      shader_uniform_set_mat4f(p_obj->shader_chunks, "u_MVP", mvp);

      renderer_draw(p_obj->renderer, p_obj->va_chunks, p_obj->vb_chunks,
                    p_obj->shader_chunks);
    }

    vertex_array_unbind();

    shader_bind(p_obj->shader_chunks_dirty_rect);

    {
      mat4 model;
      glm_translate_make(model, (vec3){0.0f, 0.0f, 0.0f});
      glm_translate(model, (vec3){p_obj->value_x, p_obj->value_y, 0.0f});

      mat4 mvp;
      glm_mat4_mul(p_obj->proj, p_obj->view, mvp);
      glm_mat4_mul(mvp, model, mvp);

      shader_uniform_set_mat4f(p_obj->shader_chunks_dirty_rect, "u_MVP", mvp);

      renderer_draw(p_obj->renderer, p_obj->va_chunks_dirty_rect,
                    p_obj->vb_chunks_dirty_rect,
                    p_obj->shader_chunks_dirty_rect);
    }

    vertex_array_unbind();
  }
}

static void on_ui_render(void *obj, void *context) {
  PixelSimObj *p_obj = (PixelSimObj *)obj;

  if (nk_button_label(context, "Toggle Debug")) {
    /* event handling */
    p_obj->show_chunks = !p_obj->show_chunks;
    p_obj->show_chunks_clear = 1;
  }

  // /* custom widget pixel width */
  // nk_layout_row_begin(context, NK_STATIC, 25, 3);
  // {
  //   nk_layout_row_push(context, 20);
  //   nk_label(context, "X:", NK_TEXT_LEFT);
  //   nk_layout_row_push(context, 30);
  //   char val[10];
  //   gcvt(p_obj->value_x, 9, val);
  //   nk_label(context, val, NK_TEXT_LEFT);
  //   nk_layout_row_push(context, 110);
  //   nk_slider_float(context, -100.0f, &p_obj->value_x, 100.0f, 1.0f);
  // }
  // nk_layout_row_end(context);
  //
  // /* custom widget pixel width */
  // nk_layout_row_begin(context, NK_STATIC, 25, 3);
  // {
  //   nk_layout_row_push(context, 20);
  //   nk_label(context, "Y:", NK_TEXT_LEFT);
  //   nk_layout_row_push(context, 30);
  //   char val[10];
  //   gcvt(p_obj->value_y, 9, val);
  //   nk_label(context, val, NK_TEXT_LEFT);
  //   nk_layout_row_push(context, 110);
  //   nk_slider_float(context, -100.0f, &p_obj->value_y, 100.0f, 1.0f);
  // }
  // nk_layout_row_end(context);
}

static void on_free(void *scene) {
  printf("Pixel Sim On Free\n");
  Scene *scene_p = (Scene *)scene;
  PixelSimObj *obj = (PixelSimObj *)scene_p->obj;

  vertex_array_free(obj->va_cells);
  vertex_buffer_free(obj->vb_cells);
  index_buffer_free(obj->ib_cells);
  vertex_buffer_layout_free(obj->layout_cells);
  vertex_array_free(obj->va_chunks);
  vertex_buffer_free(obj->vb_chunks);
  index_buffer_free(obj->ib_chunks);
  vertex_buffer_layout_free(obj->layout_chunks);
  vertex_array_free(obj->va_chunks_dirty_rect);
  vertex_buffer_free(obj->vb_chunks_dirty_rect);
  index_buffer_free(obj->ib_chunks_dirty_rect);
  vertex_buffer_layout_free(obj->layout_chunks_dirty_rect);
  shader_free(obj->shader_cells);
  shader_free(obj->shader_chunks);
  shader_free(obj->shader_chunks_dirty_rect);
  texture_free(obj->texture_air);
  texture_free(obj->texture_water);
  texture_free(obj->texture_sand);
  renderer_free(obj->renderer);
  free(obj->chunks);
  thpool_destroy(obj->threadpool);
  free(obj);
  free(scene);
}

Scene *scene_pixel_sim_init() {
  Scene *scene = malloc(sizeof(Scene));

  scene->on_update = &on_update;
  scene->on_render = &on_render;
  scene->on_ui_render = &on_ui_render;
  scene->on_free = &on_free;

  PixelSimObj *obj = malloc(sizeof(PixelSimObj));
  scene->obj = obj;

  obj->value_x = 0.0f;
  obj->value_y = 0.0f;

  obj->show_chunks = 0;
  obj->show_chunks_clear = 0;

  obj->threadpool = thpool_init(NUM_THREADS);

  obj->renderer = renderer_create();

  renderer_set_clear_color(obj->renderer, (float[]){0.2f, 0.3f, 0.3f, 1.0f});

  // const float horizontal_offset =
  //     (WINDOW_WIDTH / 2.0f) - (SIM_WIDTH_PIXEL / 2.0f);
  // const float vertical_offset = 50.0f;

  obj->chunks = calloc(SIM_CHUNK_COUNT, sizeof(Chunk));

  obj->args = calloc(SIM_CHUNK_COUNT, sizeof(struct sim_args));

  // Cells setup

  obj->shader_cells = shader_create("resources/shaders/pixel-sim-cell.glsl");
  shader_bind(obj->shader_cells);

  // Create and bind a Vertex Array Object
  obj->va_cells = vertex_array_create();

  // Create and bind a Vertex Buffer Object
  obj->vb_cells =
      vertex_buffer_create(SIM_WIDTH * SIM_HEIGHT * QUAD_NUMBER_OF_VERTICES);

  obj->layout_cells = vertex_buffer_layout_create();
  vertex_buffer_layout_push_float(obj->layout_cells, 3);
  vertex_buffer_layout_push_float(obj->layout_cells, 4);
  vertex_buffer_layout_push_float(obj->layout_cells, 2);
  vertex_buffer_layout_push_float(obj->layout_cells, 1);
  vertex_array_add_buffer(obj->va_cells, obj->vb_cells, obj->layout_cells);

  // Create and bind a Index Buffer Object
  // obj->ib = index_buffer_create(indices, 6 * 3);
  obj->ib_cells = index_buffer_create_quad(SIM_WIDTH * SIM_HEIGHT);

  glm_ortho(0.0f, WINDOW_WIDTH, 0.0f, WINDOW_HEIGHT, -1.0f, 1.0f, obj->proj);

  glm_mat4_identity(obj->view);
  glm_translate(obj->view, (vec3){0.0f, 0.0f, 0.0f});

  obj->texture_air = texture_create("resources/textures/air.png");
  texture_bind(obj->texture_air, 0);

  obj->texture_water = texture_create("resources/textures/water.png");
  texture_bind(obj->texture_water, 1);

  obj->texture_sand = texture_create("resources/textures/sand.png");
  texture_bind(obj->texture_sand, 2);

  int samplers[3] = {0, 1, 2};
  shader_uniform_set_1iv(obj->shader_cells, "u_Texture", 3, samplers);

  vertex_buffer_clear(obj->vb_cells);

  for (int chunk_id = 0; chunk_id < SIM_CHUNK_COUNT; chunk_id++) {
    Chunk *current_chunk = &obj->chunks[chunk_id];
    current_chunk->dirty_bit = 0;
    current_chunk->had_changed = 0;
    const int chunk_x_location = chunk_id % SIM_CHUNK_WIDTH;
    const int chunk_y_location = chunk_id / SIM_CHUNK_WIDTH;
    Quad *first_element = NULL;
    // printf("Chunk id: %d, x: %d, y: %d\n", chunk_id, chunk_x_location,
    //        chunk_y_location);
    for (int y = 0; y < SIM_CHUNK_SIZE; y++) {
      const int chunk_vertical_offset =
          chunk_y_location * SIM_CHUNK_SIZE * SIM_CELL_SIZE;
      for (int x = 0; x < SIM_CHUNK_SIZE; x++) {
        const int chunk_horizontal_offset =
            chunk_x_location * SIM_CHUNK_SIZE * SIM_CELL_SIZE;
        Quad q = quad_create(
            HORIZONTAL_OFFSET + chunk_horizontal_offset + x * SIM_CELL_SIZE,
            VERTICAL_OFFSET + chunk_vertical_offset + y * SIM_CELL_SIZE, 0.0f,
            SIM_CELL_SIZE, SIM_CELL_SIZE, 0.0f, (Color){0.0f, 0.0f, 0.0f, 1.0f},
            0);
        Quad *element = vertex_buffer_push_quad(obj->vb_cells, &q);
        if (first_element == NULL) {
          first_element = element;
          current_chunk->first_element = first_element;
        }
      }
    }
  }

  // for (int j = 0; j < SIM_HEIGHT; j++) {
  //   for (int i = 0; i < SIM_WIDTH; i++) {
  //     Quad q =
  //         quad_create(horizontal_offset + i * SIM_CELL_SIZE,
  //                     50.0f + j * SIM_CELL_SIZE, 0.0f, SIM_CELL_SIZE,
  //                     SIM_CELL_SIZE, 0.0f, (Color){0.0f, 0.0f, 0.0f, 1.0f},
  //                     0);
  //     vertex_buffer_push_quad(obj->vb_cells, &q);
  //   }
  // }

  vertex_buffer_flush(obj->vb_cells);

  // Unbind the VBO and VAO
  vertex_array_unbind();
  vertex_buffer_unbind();
  index_buffer_unbind();

  // Chunks setup

  obj->shader_chunks =
      shader_create("resources/shaders/pixel-sim-chunk-border.glsl");
  shader_bind(obj->shader_chunks);

  // Create and bind a Vertex Array Object
  obj->va_chunks = vertex_array_create();

  // Create and bind a Vertex Buffer Object
  obj->vb_chunks = vertex_buffer_create(SIM_CHUNK_WIDTH * SIM_CHUNK_HEIGHT *
                                        SHAPE_BOX_NUMBER_OF_VERTICES);

  obj->layout_chunks = vertex_buffer_layout_create();
  vertex_buffer_layout_push_float(obj->layout_chunks, 3);
  vertex_buffer_layout_push_float(obj->layout_chunks, 4);
  vertex_buffer_layout_push_float(obj->layout_chunks, 2);
  vertex_buffer_layout_push_float(obj->layout_chunks, 1);
  vertex_array_add_buffer(obj->va_chunks, obj->vb_chunks, obj->layout_chunks);

  // Create and bind a Index Buffer Object
  // obj->ib = index_buffer_create(indices, 6 * 3);
  obj->ib_chunks = index_buffer_create_quad(
      (SIM_CHUNK_WIDTH * SIM_CHUNK_HEIGHT * SHAPE_BOX_NUMBER_OF_VERTICES));

  glm_ortho(0.0f, WINDOW_WIDTH, 0.0f, WINDOW_HEIGHT, -1.0f, 1.0f, obj->proj);

  glm_mat4_identity(obj->view);
  glm_translate(obj->view, (vec3){0.0f, 0.0f, 0.0f});

  vertex_buffer_clear(obj->vb_chunks);

  for (int j = 0; j < SIM_CHUNK_HEIGHT; j++) {
    for (int i = 0; i < SIM_CHUNK_WIDTH; i++) {
      // Quad q = quad_create(50.0f + i * size, 50.0f + j * size, 0.0f, size,
      // size,
      //                      0.0f, (Color){0.0f, 0.0f, 0.0f, 1.0f}, 0);
      ShapeBox box = shape_box_create(
          HORIZONTAL_OFFSET + i * SIM_CHUNK_SIZE_PIXEL,
          VERTICAL_OFFSET + j * SIM_CHUNK_SIZE_PIXEL, SIM_CHUNK_SIZE_PIXEL,
          SIM_CHUNK_SIZE_PIXEL, 1, (Color){1, 0, 0, 0}, 0);
      vertex_buffer_push(obj->vb_chunks, (Vertex *)&box,
                         SHAPE_BOX_NUMBER_OF_VERTICES);
    }
  }

  vertex_buffer_flush(obj->vb_chunks);

  // Unbind the VBO and VAO
  vertex_array_unbind();
  vertex_buffer_unbind();
  index_buffer_unbind();

  // Chunks dirty rectangles setup

  obj->shader_chunks_dirty_rect =
      shader_create("resources/shaders/pixel-sim-chunk-border.glsl");
  shader_bind(obj->shader_chunks_dirty_rect);

  // Create and bind a Vertex Array Object
  obj->va_chunks_dirty_rect = vertex_array_create();

  // Create and bind a Vertex Buffer Object
  obj->vb_chunks_dirty_rect = vertex_buffer_create(
      SIM_CHUNK_WIDTH * SIM_CHUNK_HEIGHT * SHAPE_BOX_NUMBER_OF_VERTICES);

  obj->layout_chunks_dirty_rect = vertex_buffer_layout_create();
  vertex_buffer_layout_push_float(obj->layout_chunks_dirty_rect, 3);
  vertex_buffer_layout_push_float(obj->layout_chunks_dirty_rect, 4);
  vertex_buffer_layout_push_float(obj->layout_chunks_dirty_rect, 2);
  vertex_buffer_layout_push_float(obj->layout_chunks_dirty_rect, 1);
  vertex_array_add_buffer(obj->va_chunks_dirty_rect, obj->vb_chunks_dirty_rect,
                          obj->layout_chunks_dirty_rect);

  // Create and bind a Index Buffer Object
  // obj->ib = index_buffer_create(indices, 6 * 3);
  obj->ib_chunks_dirty_rect = index_buffer_create_quad(
      (SIM_CHUNK_WIDTH * SIM_CHUNK_HEIGHT * SHAPE_BOX_NUMBER_OF_VERTICES));

  vertex_buffer_clear(obj->vb_chunks_dirty_rect);

  for (int chunk_y = 0; chunk_y < SIM_CHUNK_HEIGHT; chunk_y++) {
    for (int chunk_x = 0; chunk_x < SIM_CHUNK_WIDTH; chunk_x++) {
      // Quad q = quad_create(50.0f + i * size, 50.0f + j * size, 0.0f, size,
      // size,
      //                      0.0f, (Color){0.0f, 0.0f, 0.0f, 1.0f}, 0);
      ShapeBox box =
          shape_box_create(HORIZONTAL_OFFSET + chunk_x * SIM_CHUNK_SIZE_PIXEL,
                           VERTICAL_OFFSET + chunk_y * SIM_CHUNK_SIZE_PIXEL,
                           SIM_CHUNK_SIZE_PIXEL, SIM_CHUNK_SIZE_PIXEL, 1,
                           (Color){0, 0, 0, 0}, 0);
      vertex_buffer_push(obj->vb_chunks_dirty_rect, (Vertex *)&box,
                         SHAPE_BOX_NUMBER_OF_VERTICES);

      const unsigned int chunk_id = (chunk_y * SIM_CHUNK_WIDTH) + chunk_x;
      Chunk *chunk = &obj->chunks[chunk_id];

      chunk->min_x = chunk_x * SIM_CHUNK_SIZE;
      chunk->max_x = (chunk_x + 1) * SIM_CHUNK_SIZE;
      chunk->min_y = chunk_y * SIM_CHUNK_SIZE;
      chunk->max_y = (chunk_y + 1) * SIM_CHUNK_SIZE;

      struct sim_args *args = &obj->args[chunk_id];
      args->chunk_x = chunk_x;
      args->chunk_y = chunk_y;
      args->chunk_id = chunk_id;
      args->p_obj = obj;
    }
  }

  vertex_buffer_flush(obj->vb_chunks_dirty_rect);

  // Unbind the VBO and VAO
  vertex_array_unbind();
  vertex_buffer_unbind();
  index_buffer_unbind();

  // insert_rule((SimNeighborhood){SAND, AIR, AIR, AIR}, ACTION_SWAP_DOWN);

  printf("Pixel Sim Init\n");

  // InstructionInfo instructions_p[TYPE_COUNT] = {
  //     // AIR
  //     {1,
  //      (Instruction[]){
  //          {DIR_ANY, {ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE}}}},
  //
  //     // WATER
  //     {1,
  //      (Instruction[]){
  //          {DIR_ANY, {ACTION_NONE, ACTION_NONE, ACTION_NONE, ACTION_NONE}}}},
  //
  //     // SAND
  //     {2, (Instruction[]){(Instruction){DIR_DOWN,
  //                                       {ACTION_SWAP_DOWN, ACTION_SWAP_DOWN,
  //                                        ACTION_GOTO_NEXT, ACTION_NONE}},
  //                         (Instruction){DIR_LEFT_DOWN,
  //                                       {ACTION_SWAP_DOWN, ACTION_SWAP_DOWN,
  //                                        ACTION_GOTO_NEXT, ACTION_NONE}}}},
  //
  //     // OUTOFBOUNDS
  //     {1,
  //      (Instruction[]){
  //          {DIR_ANY, {ACTION_NONE, ACTION_NONE, ACTION_NONE,
  //          ACTION_NONE}}}}};

  // FIX: Hey dus dit ben ik van voor de reis naar Dublin.
  // Dus het probleem is dat de geneste arrays niet worden overgecopied. Om dit
  // op te lossen kunt ge eigenlijk beter gewoon een functie schrijven die en
  // instuctie toevoegd door em te mallocen.
  // No clue hoe maar succes.

  // instructions = instructions_p;
  // memcpy(instructions, instructions_p, sizeof(InstructionInfo) * 4);
  // printf("debug: %d\n", instructions[TYPE_AIR].instruction[0].dir);

  return scene;
}
