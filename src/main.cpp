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
    
    const char *cube_vsource = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 a_pos;\n"
        "layout (location = 1) in vec3 a_normal;\n"
        "layout (location = 2) in vec2 a_coord;\n"
        "uniform mat4 wvp;\n"
        "uniform mat4 world;\n"
        "out vec2 tex_coord;\n"
        "out vec3 normal;\n"
        "out vec3 posw;\n"
        "void main() {\n"
        "gl_Position = wvp * vec4(a_pos, 1.0);\n"
        "posw = vec3(world * vec4(a_pos, 1.0));\n"
        "normal = mat3(transpose(inverse(world))) * a_normal;\n"
        "tex_coord = a_coord;\n"
        "}\0";
    
    const char *cube_fsource = 
        "#version 330 core\n"
        "out vec4 out_color;\n"
        "in vec2 tex_coord;\n"
        "in vec3 normal;\n"
        "in vec3 posw;\n"
        "uniform sampler2D our_texture;\n"
        "uniform vec3 object_color;\n"
        "uniform vec3 light_color;\n"
        "uniform vec3 light_pos;\n"
        "void main() {\n"
        "vec3 norm = normalize(normal);\n"
        "vec3 light_dir = normalize(light_pos - posw);\n"
        "float diff = max(dot(norm, light_dir), 0.0);\n"
        "vec3 diffuse = diff * light_color;\n"
        "float ambient_strength = 0.1;\n"
        "vec3 ambient = ambient_strength * light_color;\n"
        "vec3 lighting = (ambient + diffuse) * object_color;\n"
        "vec4 tex_color = texture(our_texture, tex_coord);\n"
        "vec4 result = tex_color * vec4(lighting, 1.0);\n"
        "out_color = result;\n"
        "}\0";
   

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

    GLuint cube_program = glCreateProgram();
    { 
        int status = 0;
        int n;
        char log[512];
        GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshader, 1, &cube_vsource, nullptr);
        glCompileShader(vshader);
        glGetShaderiv(vshader, GL_COMPILE_STATUS, &status);
        if (!status) {
            glGetShaderInfoLog(vshader, 512, &n, log);
            printf("Failed to compile vertex shader!\n%s", log);
        }
        
        GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fshader, 1, &cube_fsource, nullptr);
        glCompileShader(fshader);
        glGetShaderiv(vshader, GL_COMPILE_STATUS, &status);
        if (!status) {
            glGetShaderInfoLog(fshader, 512, &n, log);
            printf("Failed to compile fragment shader!\n%s", log);
        }
        
        glAttachShader(cube_program, vshader);
        glAttachShader(cube_program, fshader);
        glLinkProgram(cube_program);
        glDeleteShader(vshader);
        glDeleteShader(fshader);
    }
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


        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
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
       
        int world_loc = glGetUniformLocation(cube_program, "world");
        int wvp_loc = glGetUniformLocation(cube_program, "wvp");
        int light_col_loc = glGetUniformLocation(cube_program, "light_color");
        int object_col_loc = glGetUniformLocation(cube_program, "object_color");
        int light_pos_loc = glGetUniformLocation(cube_program, "light_pos");

        glUniformMatrix4fv(world_loc, 1, GL_FALSE, glm::value_ptr(world));
        glUniformMatrix4fv(wvp_loc, 1, GL_FALSE, glm::value_ptr(wvp));

        glUniform3f(light_col_loc,  1.0f, 1.0f, 1.0f);
        glUniform3f(object_col_loc, 0.88f, 0.8f, 0.8f);
        glUniform3f(light_pos_loc,  1.2f, 1.0f, 2.0f);

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
