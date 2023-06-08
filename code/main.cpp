#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h> 

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

GLuint gl_load_skymap(std::vector<char*> face_textures) {
    stbi_set_flip_vertically_on_load(false);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    int width, height, n;
    for (unsigned int i = 0; i < face_textures.size(); i++) {
        unsigned char *tex_data = stbi_load(face_textures[i], &width, &height, &n, 4);
        assert(tex_data);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
        stbi_image_free(tex_data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    return texture;
}

GLuint gl_texture_create(const char *texture_path) {
    stbi_set_flip_vertically_on_load(true);
    int tex_width, tex_height, n;
    unsigned char *tex_data = stbi_load(texture_path, &tex_width, &tex_height, &n, 4);
    if (tex_data == NULL) {
        printf("Failed to load texture: %s\n", texture_path);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)tex_data);

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(tex_data);
    return texture;
}


GLuint gl_shader_create(const char *vertex_src, const char *frag_src) {
    GLuint shader = glCreateProgram();
    int status = 0;
    int n;
    char log[512] = {};

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vertex_src, nullptr);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &status);
    if (!status) {
        printf("Failed to compile vertex shader!\n");
    }

    glGetShaderInfoLog(vshader, 512, &n, log);
    if (n > 0) {
        printf("Error in vertex shader!\n");
        printf(log);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &frag_src, nullptr);
    glCompileShader(fshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &status);
    if (!status) {
        printf("Failed to compile fragment shader!\n");
    }
    
    glGetShaderInfoLog(fshader, 512, &n, log);
    if (n > 0) {
        printf("Error in fragment shader!\n");
        printf(log);
    }

    glAttachShader(shader, vshader);
    glAttachShader(shader, fshader);
    glLinkProgram(shader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return shader;
}

GLuint gl_shader_create_from_file(const char *vertex_path, const char *fragment_path) {
    Platform_File vertex_file = read_file(vertex_path);
    Platform_File fragment_file = read_file(fragment_path);
    GLuint shader = gl_shader_create((char *)vertex_file.contents, (char *)fragment_file.contents);
    return shader;
}

int main(int argc, char **argv) {
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
    
    float skybox_vertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

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

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
                            
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
                            
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
                            
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
                            
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
                            
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
    };

    // color
    GLuint color_vao;
    glGenVertexArrays(1, &color_vao);
    glBindVertexArray(color_vao);

    GLuint color_vbo;
    glGenBuffers(1, &color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, color_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    // cube 
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

    // sky map
    GLuint skymap_vao;
    glGenVertexArrays(1, &skymap_vao);
    glBindVertexArray(skymap_vao);

    GLuint skymap_vbo;
    glGenBuffers(1, &skymap_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, skymap_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), skybox_vertices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    
    // shaders
    
    GLuint color_shader = gl_shader_create_from_file("color_v.glsl", "color_f.glsl");
    GLuint cube_shader  = gl_shader_create_from_file("cube_v.glsl", "cube_f.glsl");
    GLuint skymap_shader = gl_shader_create_from_file("skymap_v.glsl", "skymap_f.glsl");
    
    GLuint diffuse_map = gl_texture_create("data/container2.png");
    GLuint specular_map = gl_texture_create("data/container2_specular.png");

    std::vector<char*> faces;
    faces.push_back("data/skybox/right.jpg");
    faces.push_back("data/skybox/left.jpg");
    faces.push_back("data/skybox/top.jpg");
    faces.push_back("data/skybox/bottom.jpg");
    faces.push_back("data/skybox/front.jpg");
    faces.push_back("data/skybox/back.jpg");

    GLuint sky_map = gl_load_skymap(faces);

    float fov = 45.0f;
    delta_time = 0.0f;
    float last_frame = 0.0f;

    glEnable(GL_DEPTH_TEST);
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
        glm::mat4 wvp = glm::mat4(1.0f);

        glm::vec3 light_pos;
        light_pos.x = 2.0f * (float)glm::cos(glfwGetTime());
        light_pos.y = 1.0f;
        light_pos.z = 2.0f * (float)glm::sin(glfwGetTime());

        glDepthFunc(GL_LEQUAL);
        glUseProgram(skymap_shader);
        glm::mat4 sky_view = glm::mat4(glm::mat3(view));
        glUniformMatrix4fv(glGetUniformLocation(skymap_shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(skymap_shader, "view"), 1, GL_FALSE, glm::value_ptr(sky_view));

        glBindVertexArray(skymap_vao);
        glBindTexture(GL_TEXTURE_CUBE_MAP, sky_map);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        glBindVertexArray(cube_vao);
        glUseProgram(cube_shader);

        int world_loc = glGetUniformLocation(cube_shader, "world");
        int wvp_loc = glGetUniformLocation(cube_shader, "wvp");
        int eye_pos_loc = glGetUniformLocation(cube_shader, "eye_pos");

        glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f);
        glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
        glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);

        glUniform3f(glGetUniformLocation(cube_shader, "dir_source.direction"), 0.2f, -0.3f, 0.5f);

        glUniform1f(glGetUniformLocation(cube_shader, "spot_source.cut_off"), glm::cos(glm::radians(12.5f)));
        glUniform1f(glGetUniformLocation(cube_shader, "spot_source.outer_cut_off"), glm::cos(glm::radians(17.5f)));
        glUniform3fv(glGetUniformLocation(cube_shader, "spot_source.position"), 1, glm::value_ptr(cam_pos));
        glUniform3fv(glGetUniformLocation(cube_shader, "spot_source.direction"), 1, glm::value_ptr(cam_front));


        glUniform3fv(glGetUniformLocation(cube_shader, "point_source.position"), 1, glm::value_ptr(light_pos));
        glUniform1f(glGetUniformLocation(cube_shader, "point_source.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(cube_shader, "point_source.linear"), 0.7f);
        glUniform1f(glGetUniformLocation(cube_shader, "point_source.quadratic"), 1.8f);


        glUniform3fv(glGetUniformLocation(cube_shader, "dir_source.ambient"), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(cube_shader, "dir_source.diffuse"), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(cube_shader, "dir_source.specular"), 1, glm::value_ptr(specular));       

        glUniform3fv(glGetUniformLocation(cube_shader, "point_source.ambient"), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(cube_shader, "point_source.diffuse"), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(cube_shader, "point_source.specular"), 1, glm::value_ptr(specular));

        glUniform3fv(glGetUniformLocation(cube_shader, "spot_source.ambient"), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(cube_shader, "spot_source.diffuse"), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(cube_shader, "spot_source.specular"), 1, glm::value_ptr(specular));

        glUniform1i(glGetUniformLocation(cube_shader, "material.diffuse_map"), 0);
        glUniform1i(glGetUniformLocation(cube_shader, "material.specular_map"), 1);
        glUniform1f(glGetUniformLocation(cube_shader, "material.shininess"), 32.0f);

        glUniform3fv(eye_pos_loc, 1, glm::value_ptr(cam_pos));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuse_map);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specular_map);

        glm::vec3 positions[4] = {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 2.0f, 0.3f),
            glm::vec3(1.4f, 1.3f, -1.0f),
            glm::vec3(2.2f, 1.9f, 1.0f),
        };
        for (int i = 0; i < 4; i++) {
            world = glm::mat4(1.0f);
            world = glm::translate(world, positions[i]);
            wvp = projection * view * world;
            glUniformMatrix4fv(world_loc, 1, GL_FALSE, glm::value_ptr(world));
            glUniformMatrix4fv(wvp_loc, 1, GL_FALSE, glm::value_ptr(wvp));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        glBindVertexArray(color_vao);
        glUseProgram(color_shader);

        glUniform3f(glGetUniformLocation(color_shader, "color"), 1.0f, 1.0f, 1.0f);

        world = glm::mat4(1.0f);
        world = glm::translate(world, light_pos);
        wvp = projection * view * world;
        glUniformMatrix4fv(glGetUniformLocation(color_shader, "wvp"), 1, GL_FALSE, glm::value_ptr(wvp));

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
