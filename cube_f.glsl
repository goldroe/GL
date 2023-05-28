#version 330 core
in vec2 tex_coord;
in vec3 normal;
in vec3 posh;

uniform sampler2D diffuse_map;
uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 eye_pos;
out vec4 out_color;

void main() {
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - posh);
    vec3 eye_dir = normalize(eye_pos - posh);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;

    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * light_color;

    float specular_strength = 0.5;
    float spec = pow(max(dot(eye_dir, reflect_dir), 0.0), 32);
    vec3 specular = spec * specular_strength * light_color;

    vec3 lighting = (ambient + diffuse + specular) * object_color;
    vec4 tex_color = texture(diffuse_map, tex_coord);
    vec4 result = tex_color * vec4(lighting, 1.0);
    out_color = result;
}
