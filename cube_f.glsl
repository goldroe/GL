#version 330 core
in vec2 tex_coord;
in vec3 normal;
in vec3 posh;

out vec4 out_color;

struct Material {
    sampler2D diffuse_map;
    sampler2D specular_map;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

uniform vec3 eye_pos;

void main() {
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light.position - posh);
    vec3 eye_dir = normalize(eye_pos - posh);
    vec3 reflect_dir = reflect(-light_dir, norm);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse_map, tex_coord));

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse_map, tex_coord));

    float spec = pow(max(dot(eye_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular_map, tex_coord));

    vec3 lighting = (ambient + diffuse + specular);
    vec4 result = vec4(lighting, 1.0);
    out_color = result;
}
