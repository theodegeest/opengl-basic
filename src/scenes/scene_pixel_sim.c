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
#include <time.h>
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

// typedef enum {
//   DIR_DOWN,
//   DIR_LEFT_DOWN,
//   DIR_RIGHT_DOWN,
//   DIR_ANY,
//   DIR_COUNT
// } Direction;

static char inside_boundaries(int x, int y) {
  return x >= 0 && x < SIM_WIDTH && y >= 0 && y < SIM_HEIGHT;
}

static int chunk_get_id(int x, int y) {
  const int chunk_x = x / SIM_CHUNK_SIZE;
  const int chunk_y = y / SIM_CHUNK_SIZE;
  return (chunk_y * SIM_CHUNK_WIDTH) + chunk_x;
}

static Chunk *chunk_get_from_id(PixelSimObj *obj, int chunk_id) {
  return &obj->chunks[chunk_id];
}

static Chunk *chunk_get(PixelSimObj *obj, int x, int y) {
  return chunk_get_from_id(obj, chunk_get_id(x, y));
}

static Quad *cell_get_from_chunk(PixelSimObj *obj, Chunk *chunk, int x, int y) {
  if (!inside_boundaries(x, y)) {
    return NULL;
  }

  const int chunk_x = x % SIM_CHUNK_SIZE;
  const int chunk_y = y % SIM_CHUNK_SIZE;
  const int chunk_index = (chunk_y * SIM_CHUNK_SIZE) + chunk_x;
  return &chunk->first_element[chunk_index];
}

static SimType get_type_from_texture_id(float texture_id) {
  return (SimType)texture_id;
}

static float get_texture_id(SimType type) { return (float)type; }

static SimType get_type(int x, int y, Quad *q) {
  if (!inside_boundaries(x, y)) {
    return TYPE_OUTOFBOUNDS;
  }

  return get_type_from_texture_id(quad_texture_id_get(q));
}

static void set_type(int x, int y, Quad *q, SimType type) {
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

static void set_dirty_from_chunk(PixelSimObj *obj, Chunk *chunk, int x, int y) {
  chunk->dirty_bit = 1;
  keep_active(chunk, x, y);
}

static void set_dirty_from_chunk_safe(PixelSimObj *obj, Chunk *chunk, int x,
                                      int y) {
  if (inside_boundaries(x, y)) {
    set_dirty_from_chunk(obj, chunk, x, y);
  }
}

static void set_type_and_dirty_from_chunk(PixelSimObj *obj, Chunk *chunk, int x,
                                          int y, Quad *q, SimType type) {
  set_type(x, y, q, type);
  set_dirty_from_chunk(obj, chunk, x, y);
}

// Returns 0 when no changes, returns 1 when something has changed
static unsigned char update_sim_sand(PixelSimObj *obj, int chunk_id_q1,
                                     Chunk *chunk_q1, int x, int y, Quad *q1) {
  unsigned char changed = 0;

  // Check under
  int chunk_id_q2 = chunk_get_id(x, y - 1);
  Chunk *chunk_q2;
  if (chunk_id_q1 == chunk_id_q2) {
    chunk_q2 = chunk_q1;
  } else {
    chunk_q2 = chunk_get_from_id(obj, chunk_id_q2);
  }

  Quad *q2 = cell_get_from_chunk(obj, chunk_q2, x, y - 1);

  switch (get_type(x, y - 1, q2)) {
  case TYPE_AIR:
    set_type(x, y, q1, TYPE_AIR);
    set_type_and_dirty_from_chunk(obj, chunk_q2, x, y - 1, q2, TYPE_SAND);
    changed = 1;
    break;
  case TYPE_WATER:
    set_type(x, y, q1, TYPE_WATER);
    set_type_and_dirty_from_chunk(obj, chunk_q2, x, y - 1, q2, TYPE_SAND);
    changed = 1;
    break;
  case TYPE_SAND:
    // Check left
    chunk_id_q2 = chunk_get_id(x - 1, y - 1);
    if (chunk_id_q1 == chunk_id_q2) {
      chunk_q2 = chunk_q1;
    } else {
      chunk_q2 = chunk_get_from_id(obj, chunk_id_q2);
    }
    q2 = cell_get_from_chunk(obj, chunk_q2, x - 1, y - 1);
    switch (get_type(x - 1, y - 1, q2)) {
    case TYPE_AIR:
      set_type(x, y, q1, TYPE_AIR);
      set_type_and_dirty_from_chunk(obj, chunk_q2, x - 1, y - 1, q2, TYPE_SAND);
      changed = 1;
      break;
    case TYPE_WATER:
      set_type(x, y, q1, TYPE_WATER);
      set_type_and_dirty_from_chunk(obj, chunk_q2, x - 1, y - 1, q2, TYPE_SAND);
      changed = 1;
      break;
    case TYPE_SAND:
      // Check right
      chunk_id_q2 = chunk_get_id(x + 1, y - 1);
      if (chunk_id_q1 == chunk_id_q2) {
        chunk_q2 = chunk_q1;
      } else {
        chunk_q2 = chunk_get_from_id(obj, chunk_id_q2);
      }
      q2 = cell_get_from_chunk(obj, chunk_q2, x + 1, y - 1);
      switch (get_type(x + 1, y - 1, q2)) {
      case TYPE_AIR:
        set_type(x, y, q1, TYPE_AIR);
        set_type_and_dirty_from_chunk(obj, chunk_q2, x + 1, y - 1, q2,
                                      TYPE_SAND);
        changed = 1;
        break;
      case TYPE_WATER:
        set_type(x, y, q1, TYPE_WATER);
        set_type_and_dirty_from_chunk(obj, chunk_q2, x + 1, y - 1, q2,
                                      TYPE_SAND);
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
    int chunk_id_q2 = chunk_get_id(x - 1, y - 1);
    if (chunk_id_q1 == chunk_id_q2) {
      set_dirty_from_chunk_safe(obj, chunk_q1, x - 1, y - 1);
    } else {
      set_dirty_from_chunk_safe(obj, chunk_get_from_id(obj, chunk_id_q2), x - 1,
                                y - 1);
    }

    chunk_id_q2 = chunk_get_id(x - 1, y + 1);
    if (chunk_id_q1 == chunk_id_q2) {
      set_dirty_from_chunk_safe(obj, chunk_q1, x - 1, y + 1);
    } else {
      set_dirty_from_chunk_safe(obj, chunk_get_from_id(obj, chunk_id_q2), x - 1,
                                y + 1);
    }

    chunk_id_q2 = chunk_get_id(x + 1, y + 1);
    if (chunk_id_q1 == chunk_id_q2) {
      set_dirty_from_chunk_safe(obj, chunk_q1, x + 1, y + 1);
    } else {
      set_dirty_from_chunk_safe(obj, chunk_get_from_id(obj, chunk_id_q2), x + 1,
                                y + 1);
    }

    chunk_id_q2 = chunk_get_id(x + 1, y - 1);
    if (chunk_id_q1 == chunk_id_q2) {
      set_dirty_from_chunk_safe(obj, chunk_q1, x + 1, y - 1);
    } else {
      set_dirty_from_chunk_safe(obj, chunk_get_from_id(obj, chunk_id_q2), x + 1,
                                y - 1);
    }
  }
  return changed;
}

// Returns 0 when no changes, returns 1 when something has changed
static unsigned char update_sim(PixelSimObj *obj, int x, int y) {
  unsigned char changed = 0;
  int chunk_id = chunk_get_id(x, y);
  Chunk *chunk = chunk_get_from_id(obj, chunk_id);
  Quad *q = cell_get_from_chunk(obj, chunk, x, y);
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
    changed = update_sim_sand(obj, chunk_id, chunk, x, y, q);
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
    for (unsigned int y = min_y; y < max_y; y++) {
      for (unsigned int x = min_x; x < max_x; x++) {
        unsigned char changed = update_sim(p_obj, x, y);
        if (changed) {
          chunk_changed = 1;
          keep_active(chunk, x, y);
        }
      }
    }
    chunk->dirty_bit = chunk_changed;
    if (chunk_changed) {
      shape_box_color_set(chunk_box, (Color){1, 0, 0, 1});

      shape_box_color_set(chunk_dirty_rect_box, (Color){0, 0, 0, 1});
      shape_box_move(chunk_dirty_rect_box,
                     HORIZONTAL_OFFSET + chunk->min_x * SIM_CELL_SIZE,
                     VERTICAL_OFFSET + chunk->min_y * SIM_CELL_SIZE,
                     (chunk->max_x - chunk->min_x) * SIM_CELL_SIZE,
                     (chunk->max_y - chunk->min_y) * SIM_CELL_SIZE);
    } else {
      shape_box_color_set(chunk_box, (Color){0, 0, 0, 0});
      shape_box_color_set(chunk_dirty_rect_box, (Color){0, 0, 0, 0});
    }
  }
}

static void on_update(void *obj, float delta_time, GLFWwindow *window) {
  PixelSimObj *p_obj = (PixelSimObj *)obj;

  vertex_array_bind(p_obj->va_cells);

  // Timer *total = timer_init();
  // Timer *update_loop = timer_init();
  // Timer *mouse_action = timer_init();
  // Timer *data_flushing = timer_init();
  //
  // timer_start(total);
  // timer_start(update_loop);

  static char phase_offset = 0;

  phase_offset = (phase_offset + 1) % 4;

  for (char phase_base = 0; phase_base < 4; phase_base++) {
    char phase = (phase_base + phase_offset) % 4;
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
      }
    }

    thpool_wait(p_obj->threadpool);
  }

  // timer_stop(update_loop);
  // timer_start(mouse_action);

  double xpos, ypos;
  char sand_click = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
  char water_click = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
  if (sand_click || water_click) {
    glfwGetCursorPos(window, &xpos, &ypos);

    const double sim_x = xpos - HORIZONTAL_OFFSET;
    const double sim_y = (WINDOW_HEIGHT - ypos) - VERTICAL_OFFSET;

    if (sim_x >= 0 && sim_x < SIM_WIDTH_PIXEL && sim_y >= 0 &&
        sim_y < SIM_HEIGHT_PIXEL) {
      const int center_x = ((int)sim_x) / SIM_CELL_SIZE;
      const int center_y = ((int)sim_y) / SIM_CELL_SIZE;

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

          if (rand() % 10 == 0) {
            const int chunk_id = chunk_get_id(x, y);
            Chunk *chunk = chunk_get_from_id(p_obj, chunk_id);
            Quad *q = cell_get_from_chunk(p_obj, chunk, x, y);
            set_type_and_dirty_from_chunk(p_obj, chunk, x, y, q, type);
          }
        }
      }
    }
  }

  // timer_stop(mouse_action);
  // timer_start(data_flushing);

  const unsigned int number_of_vertices_in_chunk =
      SIM_CHUNK_CELL_COUNT * QUAD_NUMBER_OF_VERTICES;

  char change_found = 0;
  unsigned int start_index;
  unsigned int number_of_chunks;

  for (unsigned int chunk_id_y = 0; chunk_id_y < SIM_CHUNK_HEIGHT;
       chunk_id_y++) {
    for (unsigned int chunk_id_x = 0; chunk_id_x < SIM_CHUNK_WIDTH;
         chunk_id_x++) {
      const unsigned int chunk_id = (chunk_id_y * SIM_CHUNK_WIDTH) + chunk_id_x;
      Chunk *chunk = &p_obj->chunks[chunk_id];
      if (chunk->had_changed) {
        if (change_found) {
          // If a chnange had already been found
          number_of_chunks++;
        } else {
          // This is the first change so far
          change_found = 1;
          start_index = chunk_id * number_of_vertices_in_chunk;
          number_of_chunks = 1;
        }
      } else if (change_found) {
        // We had found a number of changes, hover now we have an unchanged
        // chunk
        change_found = 0;

        vertex_buffer_flush_part(p_obj->vb_cells, start_index,
                                 number_of_chunks *
                                     number_of_vertices_in_chunk);

        if (p_obj->show_chunks) {
          const unsigned int chunk_index =
              (start_index / number_of_vertices_in_chunk) *
              SHAPE_BOX_NUMBER_OF_VERTICES;
          vertex_buffer_flush_part(p_obj->vb_chunks, chunk_index,
                                   number_of_chunks *
                                       SHAPE_BOX_NUMBER_OF_VERTICES);

          vertex_buffer_flush_part(p_obj->vb_chunks_dirty_rect, chunk_index,
                                   number_of_chunks *
                                       SHAPE_BOX_NUMBER_OF_VERTICES);
        }
      }

      // printf("chunk_id: %d, index: %d, number_of_vertices: %d\n", chunk_id,
      //        index, number_of_vertices);
    }
  }

  if (p_obj->show_chunks_clear) {
    p_obj->show_chunks_clear = 0;
    vertex_buffer_flush(p_obj->vb_chunks);
    vertex_buffer_flush(p_obj->vb_chunks_dirty_rect);
  }

  // timer_stop(data_flushing);
  // timer_stop(total);
  //
  // printf("Total: %lf, update: %lf, mouse: %lf, data: %lf\n",
  //        timer_elapsed(total), timer_elapsed(update_loop),
  //        timer_elapsed(mouse_action), timer_elapsed(data_flushing));

  // vertex_buffer_flush(p_obj->vb_cells);
  // vertex_buffer_flush(p_obj->vb_chunks);
  // vertex_buffer_flush(p_obj->vb_chunks_dirty_rect);
}

static void on_render(void *obj) {
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

  srand(time(NULL));

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
  obj->ib_chunks = index_buffer_create_quad(
      (SIM_CHUNK_WIDTH * SIM_CHUNK_HEIGHT * SHAPE_BOX_NUMBER_OF_VERTICES));

  glm_ortho(0.0f, WINDOW_WIDTH, 0.0f, WINDOW_HEIGHT, -1.0f, 1.0f, obj->proj);

  glm_mat4_identity(obj->view);
  glm_translate(obj->view, (vec3){0.0f, 0.0f, 0.0f});

  vertex_buffer_clear(obj->vb_chunks);

  for (int j = 0; j < SIM_CHUNK_HEIGHT; j++) {
    for (int i = 0; i < SIM_CHUNK_WIDTH; i++) {
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
  obj->ib_chunks_dirty_rect = index_buffer_create_quad(
      (SIM_CHUNK_WIDTH * SIM_CHUNK_HEIGHT * SHAPE_BOX_NUMBER_OF_VERTICES));

  vertex_buffer_clear(obj->vb_chunks_dirty_rect);

  for (int chunk_y = 0; chunk_y < SIM_CHUNK_HEIGHT; chunk_y++) {
    for (int chunk_x = 0; chunk_x < SIM_CHUNK_WIDTH; chunk_x++) {
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

  printf("Pixel Sim Init\n");

  return scene;
}
