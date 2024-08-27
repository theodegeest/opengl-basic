#include <GLFW/glfw3.h>
#include <stdio.h>

// Function to handle errors
void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

// Function to handle key events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void) {
    // Set the error callback function
    glfwSetErrorCallback(error_callback);

    // Initialize the GLFW library
    if (!glfwInit()) {
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Set the key callback function
    glfwSetKeyCallback(window, key_callback);

const GLubyte* renderer = glGetString(GL_RENDERER); // get the GPU renderer
const GLubyte* version = glGetString(GL_VERSION);   // get the OpenGL version

printf("Renderer: %s\n", renderer);
printf("OpenGL version supported %s\n", version);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        // Render here
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a triangle
        glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 0.0f, 0.0f); // Red
            glVertex2f(-0.5f, -0.5f);
            glColor3f(0.0f, 1.0f, 0.0f); // Green
            glVertex2f( 0.5f, -0.5f);
            glColor3f(0.0f, 0.0f, 1.0f); // Blue
            glVertex2f( 0.0f,  0.5f);
        glEnd();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Clean up and exit
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
