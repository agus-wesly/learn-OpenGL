#version 330 core

uniform vec3 cameraPosition;

in vec3 Normal;
in vec3 FragmentPosition;
in vec2 TexCoords;

out vec4 FragColor;

struct Material {
    sampler2D diffuseMap;
    sampler2D  specular;
    float shininess;
};

struct Light {
    vec3  position;
    vec3  direction;
    float cutOff;
    float outerCutOff;

    vec3  ambient;
    vec3  diffuse;
    vec3  specular;

    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform Light    light;

void main()
{
    vec3 ambient = (light.ambient * texture(material.diffuseMap, TexCoords).rgb);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragmentPosition);
    float diff = max(dot(lightDir, norm), 0.0f);
    vec3 diffuse = (diff * light.diffuse * texture(material.diffuseMap, TexCoords).rgb);

    vec3 cameraDir = normalize(cameraPosition - FragmentPosition);
    vec3 reflectedDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(cameraDir, reflectedDir), 0.0f), material.shininess);
    vec3 specular = (spec * light.specular * texture(material.specular, TexCoords).rgb);

    float distance = length(light.position - FragmentPosition);
    float attenuation = 1.0f / (light.constant + (light.linear * distance) + (light.quadratic * pow(distance, 2)));

    diffuse *= attenuation;
    ambient *= attenuation;
    specular *= attenuation;

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0f, 1.0f);

    diffuse *= intensity;
    specular *= intensity;

    FragColor = vec4(diffuse + ambient + specular, 1.0f);
};
