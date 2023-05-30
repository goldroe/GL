in vec2 tex_coord;
in vec3 normal;
in vec3 posh;

out vec4 out_color;

struct Material {
    sampler2D diffuse_map;
    sampler2D specular_map;
    float shininess;
};

struct Directional_Light {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Point_Light {
    vec3  position;
    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Spot_Light {
    vec3 position;
    vec3 direction;
    float cut_off;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform vec3 eye_pos;

uniform Directional_Light dir_source;
uniform Point_Light point_source;
uniform Spot_Light spot_source;

uniform Spot_Light light;
vec3 compute_directional_light(Directional_Light light, vec3 normal, vec3 eye_dir) {
    vec3 light_dir = normalize(-light.direction);
    vec3 reflect_dir = reflect(-light_dir, normal);
    vec3 ambient = light.ambient * texture(material.diffuse_map, tex_coord).rgb;
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = diff * light.diffuse * texture(material.diffuse_map, tex_coord).rgb;
    float spec = pow(max(dot(eye_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = spec * light.specular * texture(material.specular_map, tex_coord).rgb;

    return ambient + diffuse + specular;
}

vec3 compute_point_light(Point_Light light, vec3 normal, vec3 eye_dir) {
    vec3 light_dir = normalize(light.position - posh);
    float dist = length(light.position - posh);
    vec3 reflect_dir = reflect(-light_dir, normal);
    
    vec3 ambient = light.ambient * texture(material.diffuse_map, tex_coord).rgb;
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = diff * light.diffuse * texture(material.diffuse_map, tex_coord).rgb;
    float spec = pow(max(dot(eye_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = spec * light.specular * texture(material.specular_map, tex_coord).rgb;

    float attenuation = (1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return ambient + diffuse + specular;
}

vec3 compute_spot_light(Spot_Light light, vec3 normal, vec3 eye_dir) {
    vec3 light_dir = normalize(light.position - posh);
    float theta = dot(light_dir, normalize(-light.direction));

    vec3 ambient = light.ambient * texture(material.diffuse_map, tex_coord).rgb;
    vec3 diffuse;
    vec3 specular;

    if (theta > light.cut_off) {
        float diff = max(dot(normal, light_dir), 0.0);
        diffuse = diff * light.diffuse * texture(material.diffuse_map, tex_coord).rgb;

        vec3 reflect_dir = reflect(-light_dir, normal);
        float spec = pow(max(dot(eye_dir, reflect_dir), 0.0), material.shininess);
        specular = spec * light.specular * texture(material.specular_map, tex_coord).rgb;
    } else {
        return ambient;
    }

    return ambient + diffuse + specular;
}

void main() {
    vec3 norm = normalize(normal);
    vec3 eye_dir = normalize(eye_pos - posh);

    vec3 lighting = vec3(0.0);
    lighting += compute_directional_light(dir_source, norm, eye_dir);
    lighting += compute_point_light(point_source, norm, eye_dir);
    lighting += compute_spot_light(spot_source, norm, eye_dir);

    out_color = vec4(lighting, 1.0);
}
