#version 330 core

uniform vec3 cameraPosition;

in vec3 Normal;
in vec3 FragmentPosition;

out vec4 FragColor;

struct Material {
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    float shininess;
};

struct Light {
    vec3  position;

    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
};

uniform Material material;
uniform Light    light;

void main()
{
    vec3 ambient = (light.ambient * material.ambient);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragmentPosition);
    float diff = max(dot(lightDir, norm), 0.0f);
    vec3 diffuse = (diff * light.diffuse * material.diffuse);

    vec3 cameraDir = normalize(cameraPosition - FragmentPosition);
    vec3 reflectedDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(cameraDir, reflectedDir), 0.0f), material.shininess);
    vec3 specular = (spec * light.specular * material.specular);
    vec3 result = (diffuse + ambient + specular);

    FragColor = vec4(result, 1.0f);
};
