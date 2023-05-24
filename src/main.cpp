#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h> 

#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>

#include "common.h"

const int WIDTH = 1600;
const int HEIGHT = 900;

void FrameBufferSizeCallback(GLFWwindow* Window, int Width, int Height) {
    glViewport(0, 0, Width, Height);
}

int main(int Argc, char **Argv) {
    // stbi_set_flip_vertically_on_load(true);
    if (!glfwInit()) {
        printf("Could not initialize glfw\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    GLFWwindow* Window = glfwCreateWindow(WIDTH, HEIGHT, "GL", NULL, NULL);

    if (!Window) {
        printf("Failed to create window!\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(Window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Could not initialize glad\n");
        return -1;
    }

    glfwSetFramebufferSizeCallback(Window, FrameBufferSizeCallback);
    glfwSwapInterval(1);

    const char *SquareVSource = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 Pos;\n"
        "layout (location = 1) in vec3 Col;\n"
        "out vec3 InColor;\n"
        "void main() {\n"
        "gl_Position = vec4(Pos, 1.0);"
        "InColor = Col;\n"
        "}\0";

    const char *SquareFSource = 
        "#version 330 core\n"
        "in vec3 InColor;\n"
        "out vec4 OutColor;\n"
        "void main() {\n"
        "OutColor = vec4(InColor, 1.0);\n"
        "}\0";

    float SquareVerts[] = {
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.0f,   0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f,    0.0f, 0.0f, 1.0f,

        -0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f,    1.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 1.0f,
    };

    GLuint SquareVAO;
    glGenVertexArrays(1, &SquareVAO);
    glBindVertexArray(SquareVAO);

    GLuint SquareVBO;
    glGenBuffers(1, &SquareVBO);
    glBindBuffer(GL_ARRAY_BUFFER, SquareVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(SquareVerts), SquareVerts, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

    int status = 0;
    GLuint SquareProgram = glCreateProgram();
    { 
        GLuint VShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(VShader, 1, &SquareVSource, nullptr);
        glCompileShader(VShader);
        glGetShaderiv(VShader, GL_COMPILE_STATUS, &status);
        if (!status) {
            printf("Failed to compile vertex shader!\n");
        }

        GLuint FShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(FShader, 1, &SquareFSource, nullptr);
        glCompileShader(FShader);
        glGetShaderiv(VShader, GL_COMPILE_STATUS, &status);
        if (!status) {
            printf("Failed to compile vertex shader!\n");
        }

        glAttachShader(SquareProgram, VShader);
        glAttachShader(SquareProgram, FShader);
        glLinkProgram(SquareProgram);
        glDeleteShader(VShader);
        glDeleteShader(FShader);
    }
    glUseProgram(SquareProgram);

    while (!glfwWindowShouldClose(Window)) {
        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(SquareVAO);
        glUseProgram(SquareProgram);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(Window);
        glfwPollEvents();
    }

    glfwDestroyWindow(Window);
    glfwTerminate();

    return 0;
}
