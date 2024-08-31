#include "../include/glad/glad.h"
#include <GLFW/glfw3.h>
// #include <cglm/affine-mat.h>
// #include <cglm/affine-pre.h>
// #include <cglm/cam.h>
// #include <cglm/mat4.h>
// #include <cglm/types.h>
// #include <cglm/vec3.h>
#include <stdio.h>

//----------------------nk-------------------------
// Here we define everything needed to set up the library with glfw.
// Note that nuklear_glfw_gl3.h should be included after nuklear.h.
#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION

#include "../include/Nuklear/nuklear.h"
//
#include "../include/Nuklear/demo/glfw_opengl4/nuklear_glfw_gl4.h"
// #include <nuklear\demo\glfw_opengl3\nuklear_glfw_gl3.h>
//----------------------------------------------------

#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "debug.h"
#include "index_buffer.h"
#include "renderer.h"
#include "shader.h"
#include "texture.h"
#include "vertex_array.h"
#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"

// Function to handle key events
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void) {
  // Initialize GLFW
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  // Set OpenGL version to 3.3 and profile to core
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a windowed mode window and its OpenGL context
  GLFWwindow *window =
      glfwCreateWindow(800, 600, "OpenGL with GLAD", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);

  // Load OpenGL function pointers with GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    fprintf(stderr, "Failed to initialize GLAD\n");
    return -1;
  }

  //-------------------------nk-------------------------------
  // Here we set up the context for nuklear which contains info about the
  // styles, fonts etc about the GUI We also set up the font here
  struct nk_context *context =
      nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER,
                    MAX_ELEMENT_BUFFER);
  // set up font
  struct nk_font_atlas *atlas;
  nk_glfw3_font_stash_begin(&atlas);
  nk_glfw3_font_stash_end();
  //----------------------------------------------------------

  const GLubyte *gl_renderer = glGetString(GL_RENDERER); // get the GPU renderer
  const GLubyte *version = glGetString(GL_VERSION); // get the OpenGL version

  printf("Renderer: %s\n", gl_renderer);
  printf("OpenGL version supported %s\n\n", version);

  // Set a clear color to distinguish the triangle
  GLCall(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));

  // Set the key callback
  glfwSetKeyCallback(window, key_callback);

  Shader *shader = shader_create("resources/shaders/basic.glsl");
  shader_bind(shader);

  // Define the vertices for a triangle
  float vertices[] = {100.0f, 100.0f, 0.0f,   0.0f,   0.0f,   200.0f, 100.0f,
                      0.0f,   1.0f,   0.0f,   200.0f, 200.0f, 0.0f,   1.0f,
                      1.0f,   100.0f, 200.0f, 0.0f,   0.0f,   1.0f};

  unsigned int indices[] = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  GLCall(glEnable(GL_BLEND));
  GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

  // Create and bind a Vertex Array Object
  VertexArray *va = vertex_array_create();

  // Create and bind a Vertex Buffer Object
  VertexBuffer *vb = vertex_buffer_create(vertices, sizeof(vertices));

  VertexBufferLayout *layout = vertex_buffer_layout_create();
  vertex_buffer_layout_push_float(layout, 3);
  vertex_buffer_layout_push_float(layout, 2);
  vertex_array_add_buffer(va, vb, layout);
  // glCheckError();

  // Create and bind a Index Buffer Object
  IndexBuffer *ib = index_buffer_create(indices, 6);

  mat4 proj;
  glm_ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f, proj);

  mat4 view;
  glm_mat4_identity(view);
  glm_translate(view, (vec3){100.0f, 0.0f, 0.0f});

  // glClearError();
  Texture *texture = texture_create("resources/textures/square.png");
  // glCheckError();
  texture_bind(texture, 0);
  // glClearError();
  shader_uniform_set_1i(shader, "u_Texture", 0);
  // glCheckError();

  // Unbind the VBO and VAO
  vertex_array_unbind();
  vertex_buffer_unbind();
  index_buffer_unbind();

  float r = 0.0f;
  float r_inc = 0.002f;

  Renderer *renderer = renderer_create();

  // Rendering loop
  while (!glfwWindowShouldClose(window)) {

    //--------------------------nk------------------------------
    // create a new frame for every iteration of the loop
    // here we set up the nk_window
    nk_glfw3_new_frame();

    enum { EASY, HARD };
    static int op = EASY;
    static float value_x = 0.0f;
    static float value_y = 0.0f;
    static float value_z = 0.0f;
    static int i = 20;
    if (nk_begin(context, "Nuklear Window", nk_rect(0, 0, 200, 300),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE |
                     NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE)) {
      /* fixed widget pixel width */
      nk_layout_row_static(context, 30, 80, 1);
      if (nk_button_label(context, "button")) {
        /* event handling */
        printf("Button\n");
      }

      /* fixed widget window ratio width */
      nk_layout_row_dynamic(context, 30, 2);
      if (nk_option_label(context, "easy", op == EASY))
        op = EASY;
      if (nk_option_label(context, "hard", op == HARD))
        op = HARD;

      /* custom widget pixel width */
      nk_layout_row_begin(context, NK_STATIC, 30, 3);
      {
        nk_layout_row_push(context, 20);
        nk_label(context, "X:", NK_TEXT_LEFT);
        nk_layout_row_push(context, 30);
        char val[10];
        gcvt(value_x, 9, val);
        nk_label(context, val, NK_TEXT_LEFT);
        nk_layout_row_push(context, 110);
        nk_slider_float(context, -100.0f, &value_x, 100.0f, 1.0f);
      }
      nk_layout_row_end(context);

      /* custom widget pixel width */
      nk_layout_row_begin(context, NK_STATIC, 30, 3);
      {
        nk_layout_row_push(context, 20);
        nk_label(context, "Y:", NK_TEXT_LEFT);
        nk_layout_row_push(context, 30);
        char val[10];
        gcvt(value_y, 9, val);
        nk_label(context, val, NK_TEXT_LEFT);
        nk_layout_row_push(context, 110);
        nk_slider_float(context, -100.0f, &value_y, 100.0f, 1.0f);
      }
      nk_layout_row_end(context);

      /* custom widget pixel width */
      nk_layout_row_begin(context, NK_STATIC, 30, 3);
      {
        nk_layout_row_push(context, 20);
        nk_label(context, "Z:", NK_TEXT_LEFT);
        nk_layout_row_push(context, 30);
        char val[10];
        gcvt(value_z, 9, val);
        nk_label(context, val, NK_TEXT_LEFT);
        nk_layout_row_push(context, 110);
        nk_slider_float(context, -100.0f, &value_z, 100.0f, 1.0f);
      }
      nk_layout_row_end(context);
    }
    nk_end(context);
    //------------------------------------------------------------
    // printf("%f\n", value);

    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, (vec3){value_x, value_y, value_z});

    mat4 mvp;
    glm_mul(proj, view, mvp);
    glm_mul(mvp, model, mvp);


    r += r_inc;
    if (r > 1) {
      r = 0.0f;
    }
    // Clear the screen
    // glClear(GL_COLOR_BUFFER_BIT);
    renderer_clear(renderer);

    // Use our shader program
    shader_bind(shader);

    // shader_uniform_set_4f(shader, "u_Color", r, 0.3f, 0.8f, 1.0f);
    shader_uniform_set_mat4f(shader, "u_MVP", mvp);
    // glCheckError();

    // Draw the triangle
    // vertex_array_bind(va);

    renderer_draw(renderer, va, ib, shader);
    // glClearError();
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    // glCheckError();

    vertex_array_unbind();

    //-------------------------nk------------------------------------
    // don't forget to draw your window!
    nk_glfw3_render(NK_ANTI_ALIASING_ON);
    //---------------------------------------------------------------

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }

  // Clean up
  vertex_array_free(va);
  vertex_buffer_free(vb);
  index_buffer_free(ib);
  vertex_buffer_layout_free(layout);

  shader_free(shader);
  renderer_free(renderer);
  texture_free(texture);

  //-------------------------nk------------------------------------
  nk_glfw3_shutdown();
  //-------------------------------------------------------------

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
