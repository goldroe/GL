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
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "common.h"

const int WIDTH = 1600;
const int HEIGHT = 900;

float delta_time;

glm::vec3 cam_pos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cam_front = glm::vec3(0.0f, 0.0f, -1.0);
glm::vec3 cam_up = glm::vec3(0.0f, 1.0f, 0.0f);

bool first_mouse = true;
float last_cursor_x, last_cursor_y;

float yaw = -90.0f;
float pitch = 0.0f;

struct Input {
    bool up;
    bool down;
    bool left;
    bool right;
};

Input input{};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_W) {
        input.up = (action != GLFW_RELEASE);
    } 
    if (key == GLFW_KEY_S) {
        input.down = (action != GLFW_RELEASE);
    } 
    if (key == GLFW_KEY_A) {
        input.left = (action != GLFW_RELEASE);
    }
    if (key == GLFW_KEY_D) {
        input.right = (action != GLFW_RELEASE);
    }
}


void mouse_callback(GLFWwindow* window, double cursor_x, double cursor_y) {
    if (first_mouse) {
        last_cursor_x = (float)cursor_x;
        last_cursor_y = (float)cursor_y;
        first_mouse = false;
    }

    float offset_x = (float)cursor_x - last_cursor_x;
    float offset_y = last_cursor_y - (float)cursor_y;

    float sensitivity = 0.1f;
    offset_x *= sensitivity;
    offset_y *= sensitivity;

    yaw += offset_x;
    pitch += offset_y;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cam_front = glm::normalize(dir);
    last_cursor_x = (float)cursor_x;
    last_cursor_y = (float)cursor_y;
}

void frame_buffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

struct Platform_File {
    void *contents;
    int64_t content_size;
};

int64_t get_file_size(FILE *fp) {
    int64_t size = 0;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    return size;
}

Platform_File read_file(const char *file_name) {
    Platform_File result{};
    FILE *fp = fopen(file_name, "rb");
    if (fp == NULL) {
        printf("Error opening file: %s\n", file_name);
        return result;
    }

    int64_t len = get_file_size(fp);
    char *contents = (char *)malloc((len + 1) * sizeof(char));
    fread(contents, len, sizeof(char), fp);
    contents[len] = '\0';
    fclose(fp);

    result.contents = contents;
    result.content_size = len;
    return result;
}





GLuint gl_shader_create(const char *vertex_src, const char *frag_src) {
    GLuint program = glCreateProgram();
    int status = 0;
    int n;
    char log[512];

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vertex_src, nullptr);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &status);
    if (!status) {
        printf("Failed to compile vertex shader!\n");
    } else {
        printf("Vertex Log:\n");
    }
    
    glGetShaderInfoLog(vshader, 512, &n, log);
    printf(log);

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &frag_src, nullptr);
    glCompileShader(fshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &status);
    if (!status) {
        printf("Failed to compile fragment shader!\n");
    } else {
        printf("Fragment Log:\n");
    }
    
    glGetShaderInfoLog(fshader, 512, &n, log);
    printf(log);

    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return program;
}

GLuint gl_shader_create_from_file(const char *vertex_path, const char *fragment_path) {
    Platform_File vertex_file = read_file(vertex_path);
    Platform_File fragment_file = read_file(fragment_path);
    GLuint program = gl_shader_create((char *)vertex_file.contents, (char *)fragment_file.contents);
    return program;
}

int main(int argc, char **argv) {
    // stbi_set_flip_vertically_on_load(true);
    if (!glfwInit()) {
        printf("Could not initialize glfw\n");
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "GL", NULL, NULL);
    
    if (!window) {
        printf("Failed to create window!\n");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Could not initialize glad\n");
        return -1;
    }
    
    glfwSetFramebufferSizeCallback(window, frame_buffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glfwSwapInterval(1);
    
    float cube_vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
                                                               
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,    0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,    1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,    1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,    1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,    0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,    0.0f, 0.0f,
                                                               
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
                                                               
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
                                                               
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
                                                               
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
    };
            
    GLuint cube_vao;
    glGenVertexArrays(1, &cube_vao);
    glBindVertexArray(cube_vao);
    
    GLuint cube_vbo;
    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

    GLuint cube_program = gl_shader_create_from_file("cube_v.glsl", "cube_f.glsl");
    glUseProgram(cube_program);
    
    stbi_set_flip_vertically_on_load(true);
    int tex_width, tex_height, n;
    unsigned char *tex_data = stbi_load("data/braynzar.jpg", &tex_width, &tex_height, &n, 4);

    GLuint texture_map;
    glGenTextures(1, &texture_map);
    glBindTexture(GL_TEXTURE_2D, texture_map);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)tex_data);

    glGenerateMipmap(GL_TEXTURE_2D);


    stbi_image_free(tex_data);

    glEnable(GL_DEPTH_TEST);
    float fov = 45.0f;
    delta_time = 0.0f;
    float last_frame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float current_frame = (float)glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        float cam_speed = 2.5f * delta_time;
        if (input.up) {
            cam_pos += cam_front * cam_speed;
        }
        if (input.down) {
            cam_pos -= cam_front * cam_speed;
        } 
        if (input.left) {
            cam_pos -= glm::normalize(glm::cross(cam_front, cam_up)) * cam_speed;
        }
        if (input.right) {
            cam_pos += glm::normalize(glm::cross(cam_front, cam_up)) * cam_speed;
        }


        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glm::mat4 view = glm::lookAt(cam_pos, cam_pos + cam_front, cam_up);

        glm::vec2 window_size;
        {
            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            window_size.x = (float)w;
            window_size.y = (float)h;
        }
        glm::mat4 projection = glm::perspective(glm::radians(fov), window_size.x / window_size.y, 0.1f, 100.0f);
        glm::mat4 world = glm::mat4(1.0f);
        glm::mat4 wvp = projection * view;

        glm::vec3 light_pos;
        light_pos.x = 2.0f * (float)glm::cos(glfwGetTime());
        light_pos.y = 1.0f;
        light_pos.z = 2.0f * (float)glm::sin(glfwGetTime());
       
        int world_loc = glGetUniformLocation(cube_program, "world");
        int wvp_loc = glGetUniformLocation(cube_program, "wvp");
        int eye_pos_loc = glGetUniformLocation(cube_program, "eye_pos");

        glUniformMatrix4fv(world_loc, 1, GL_FALSE, glm::value_ptr(world));
        glUniformMatrix4fv(wvp_loc, 1, GL_FALSE, glm::value_ptr(wvp));
        glUniform3fv(eye_pos_loc,   1, glm::value_ptr(cam_pos));

        glUniform3fv(glGetUniformLocation(cube_program, "light.position"), 1, glm::value_ptr(light_pos));
        glUniform3f(glGetUniformLocation(cube_program, "light.ambient"), 0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(cube_program, "light.diffuse"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(cube_program, "light.specular"), 1.0f, 1.0f, 1.0f);

        glUniform3f(glGetUniformLocation(cube_program, "material.ambient"), 1.0f, 0.5f, 0.31f);
        glUniform3f(glGetUniformLocation(cube_program, "material.diffuse"), 1.0f, 0.5f, 0.31f);
        glUniform4f(glGetUniformLocation(cube_program, "material.specular"), 0.5f, 0.5f, 0.5f, 32.0f);

        glBindVertexArray(cube_vao);
        glUseProgram(cube_program);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
