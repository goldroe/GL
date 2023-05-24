#version 330 core
layout (location = 0) in vec2 a_pos;
uniform mat4 wvp;
out vec2 posl;
void main() {
    gl_Position = wvp * vec4(a_pos.x, a_pos.y, 0.0f, 1.0f);
    posl = a_pos;
};

#version 330 core
out vec4 frag_color;
in vec2 posl;
uniform vec3 our_color;
void main() {
    if (posl.x > 0.05 && posl.x < 0.95 && posl.y > 0.05 && posl.y < 0.95) discard;
    frag_color = vec4(our_color.r, our_color.g, our_color.b, 1.0);
}

