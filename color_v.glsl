#version 330 core
layout (location = 0) in vec3 a_pos;

uniform mat4 wvp;

void main() {
    gl_Position = wvp * vec4(a_pos, 1.0);
}
