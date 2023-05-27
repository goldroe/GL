#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_coord;

uniform mat4 wvp;
uniform mat4 world;

out vec2 tex_coord;
out vec3 normal;
out vec3 posh;

void main() {
    gl_Position = wvp * vec4(a_pos, 1.0);
    posh = vec3(world * vec4(a_pos, 1.0));
    normal = mat3(transpose(inverse(world))) * a_normal;
    tex_coord = a_coord;
};
