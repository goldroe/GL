#version 330 core
in vec2 tex_coord;
in vec3 normal;
in vec3 posh;

uniform sampler2D our_texture;
uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;

out vec4 out_color;

void main() {
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - posh);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * light_color;
    vec3 lighting = (ambient + diffuse) * object_color;
    vec4 tex_color = texture(our_texture, tex_coord);
    vec4 result = tex_color * vec4(lighting, 1.0);
    out_color = result;
};
