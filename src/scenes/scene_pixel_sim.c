#include "scene_pixel_sim.h"
#include "../../include/glad/glad.h"
#include "../graphics/graphics.h"
#include "../graphics/index_buffer.h"
#include "../graphics/renderer.h"
#include "../graphics/shader.h"
#include "../graphics/texture.h"
#include "../graphics/vertex_array.h"
#include "../mesh/shape.h"
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/Nuklear/nuklear.h"

#define SIM_WIDTH 130
#define SIM_HEIGHT 60
#define SIM_CELL_SIZE 10

#define SIM_CHUNK_SIZE 25
#define SIM_CHUNK_WIDTH SIM_WIDTH / SIM_CHUNK_SIZE
#define SIM_CHUNK_HEIGHT SIM_HEIGHT / SIM_CHUNK_SIZE
#define SIM_CHUNK_SIZE_PIXEL SIM_CELL_SIZE * SIM_CHUNK_SIZE

#define SIM_WIDTH_PIXEL SIM_WIDTH *SIM_CELL_SIZE
#define SIM_HEIGHT_PIXEL SIM_HEIGHT *SIM_CELL_SIZE

typedef struct {
  VertexArray *va_cells;
  VertexBuffer *vb_cells;
  IndexBuffer *ib_cells;
  VertexBufferLayout *layout_cells;
  VertexArray *va_chunks;
  VertexBuffer *vb_chunks;
  IndexBuffer *ib_chunks;
  VertexBufferLayout *layout_chunks;
  Shader *shader_cells;
  Shader *shader_chunks;
  Texture *texture_air;
  Texture *texture_water;
  Texture *texture_sand;
  Renderer *renderer;
  float value_x;
  float value_y;
  mat4 proj;
  mat4 view;
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

static SimType get_type_from_texture_id(float texture_id) {
  return (SimType)texture_id;
}

static SimType get_type(VertexBuffer *vb, int x, int y) {
  if (x < 0 || x >= SIM_WIDTH || y < 0 || y >= SIM_HEIGHT) {
    return TYPE_OUTOFBOUNDS;
  }
  int i = SIM_WIDTH * y + x;
  Quad *q = vertex_buffer_quad_get(vb, i * 4);
  return get_type_from_texture_id(quad_texture_id_get(q));
}

static SimType get_type_from_dir(VertexBuffer *vb, int x, int y,
                                 Direction dir) {
  switch (dir) {
  case DIR_DOWN:
    return get_type(vb, x, y - 1);
  case DIR_LEFT_DOWN:
    return get_type(vb, x - 1, y - 1);
  case DIR_RIGHT_DOWN:
    return get_type(vb, x + 1, y - 1);
  case DIR_ANY:
    printf("Should not receive DIR_ANY in get_type_from_dir\n");
    return TYPE_OUTOFBOUNDS;
  case DIR_COUNT:
    printf("Should not receive DIR_ANY in get_type_from_dir\n");
    return TYPE_OUTOFBOUNDS;
  }

  return TYPE_OUTOFBOUNDS;
}

// static SimNeighborhood get_neighborhood(VertexBuffer *vb, int x, int y) {
//   return (SimNeighborhood){get_type(vb, x, y), get_type(vb, x, y - 1),
//                            get_type(vb, x - 1, y - 1),
//                            get_type(vb, x + 1, y - 1)};
// }

static void set_type(VertexBuffer *vb, int x, int y, SimType type) {
  if (x < 0 || x >= SIM_WIDTH || y < 0 || y >= SIM_HEIGHT) {
    printf("set_type out of bounds\n");
    return;
  }
  int i = SIM_WIDTH * y + x;
  Quad *q = vertex_buffer_quad_get(vb, i * 4);
  quad_texture_id_set(q, type);
}

static float get_texture_id(SimType type) { return (float)type; }

static void update_sim_sand(VertexBuffer *vb, int x, int y) {
  // Check under
  SimType under = get_type(vb, x, y - 1);
  switch (under) {
  case TYPE_AIR:
    set_type(vb, x, y, TYPE_AIR);
    set_type(vb, x, y - 1, TYPE_SAND);
    break;
  case TYPE_WATER:
    set_type(vb, x, y, TYPE_WATER);
    set_type(vb, x, y - 1, TYPE_SAND);
    break;
  case TYPE_SAND:
    // Check left
    SimType left = get_type(vb, x - 1, y - 1);
    switch (left) {
    case TYPE_AIR:
      set_type(vb, x, y, TYPE_AIR);
      set_type(vb, x - 1, y - 1, TYPE_SAND);
      break;
    case TYPE_WATER:
      set_type(vb, x, y, TYPE_WATER);
      set_type(vb, x - 1, y - 1, TYPE_SAND);
      break;
    case TYPE_SAND:
      // Check right
      SimType right = get_type(vb, x + 1, y - 1);
      switch (right) {
      case TYPE_AIR:
        set_type(vb, x, y, TYPE_AIR);
        set_type(vb, x + 1, y - 1, TYPE_SAND);
        break;
      case TYPE_WATER:
        set_type(vb, x, y, TYPE_WATER);
        set_type(vb, x + 1, y - 1, TYPE_SAND);
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
}

static void update_sim(VertexBuffer *vb, int x, int y) {
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

  SimType type = get_type(vb, x, y);
  switch (type) {
  case TYPE_AIR:
    break;
  case TYPE_WATER:
    break;
  case TYPE_SAND:
    update_sim_sand(vb, x, y);
    break;
  case TYPE_OUTOFBOUNDS:
  case TYPE_COUNT:
    break;
  }
}

static void on_update(void *obj, float delta_time) {
  PixelSimObj *p_obj = (PixelSimObj *)obj;
  // printf("Clear Color On Update\n");

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
  int total = SIM_WIDTH * SIM_HEIGHT;
  Quad *spawn_q =
      vertex_buffer_quad_get(p_obj->vb_cells, (total - SIM_WIDTH / 2) * 4);
  quad_texture_id_set(spawn_q, 2);

  for (int i = 0; i < total; i++) {
    int x = i % SIM_WIDTH;
    int y = i / SIM_WIDTH;

    // Quad *current = vertex_buffer_quad_get(p_obj->vb, i);
    // if (current == NULL) {
    //   printf("Could not get quad\n");
    // }

    update_sim(p_obj->vb_cells, x, y);
    // quad_texture_id_set(current, 1.0f);
  }

  vertex_buffer_flush(p_obj->vb_cells);
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
}

static void on_ui_render(void *obj, void *context) {
  PixelSimObj *p_obj = (PixelSimObj *)obj;

  /* custom widget pixel width */
  nk_layout_row_begin(context, NK_STATIC, 25, 3);
  {
    nk_layout_row_push(context, 20);
    nk_label(context, "X:", NK_TEXT_LEFT);
    nk_layout_row_push(context, 30);
    char val[10];
    gcvt(p_obj->value_x, 9, val);
    nk_label(context, val, NK_TEXT_LEFT);
    nk_layout_row_push(context, 110);
    nk_slider_float(context, -100.0f, &p_obj->value_x, 100.0f, 1.0f);
  }
  nk_layout_row_end(context);

  /* custom widget pixel width */
  nk_layout_row_begin(context, NK_STATIC, 25, 3);
  {
    nk_layout_row_push(context, 20);
    nk_label(context, "Y:", NK_TEXT_LEFT);
    nk_layout_row_push(context, 30);
    char val[10];
    gcvt(p_obj->value_y, 9, val);
    nk_label(context, val, NK_TEXT_LEFT);
    nk_layout_row_push(context, 110);
    nk_slider_float(context, -100.0f, &p_obj->value_y, 100.0f, 1.0f);
  }
  nk_layout_row_end(context);
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
  shader_free(obj->shader_cells);
  shader_free(obj->shader_chunks);
  texture_free(obj->texture_air);
  texture_free(obj->texture_water);
  texture_free(obj->texture_sand);
  renderer_free(obj->renderer);
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

  obj->renderer = renderer_create();

  renderer_set_clear_color(obj->renderer, (float[]){0.2f, 0.3f, 0.3f, 1.0f});

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

  int size = 10;

  for (int j = 0; j < SIM_HEIGHT; j++) {
    for (int i = 0; i < SIM_WIDTH; i++) {
      Quad q = quad_create(50.0f + i * size, 50.0f + j * size, 0.0f, size, size,
                           0.0f, (Color){0.0f, 0.0f, 0.0f, 1.0f}, 0);
      vertex_buffer_push_quad(obj->vb_cells, &q);
    }
  }

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
          50.0f + i * SIM_CHUNK_SIZE_PIXEL, 50.0f + j * SIM_CHUNK_SIZE_PIXEL,
          SIM_CHUNK_SIZE_PIXEL, SIM_CHUNK_SIZE_PIXEL, 1, (Color){1, 0, 0, 1}, 0);
      vertex_buffer_push(obj->vb_chunks, (Vertex *)&box,
                         SHAPE_BOX_NUMBER_OF_VERTICES);
    }
  }

  vertex_buffer_flush(obj->vb_chunks);

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
