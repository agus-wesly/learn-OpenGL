#version 330 core

uniform vec3 cubeColor;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

in vec3 Normal;
in vec3 FragmentPosition;

out vec4 FragColor;

uniform vec3 lightColor;

void main()
{
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPosition - FragmentPosition);
    float diffuseStrength = max(dot(lightDir, norm), 0.0f);
    vec3 diffuse = diffuseStrength * lightColor;

    const float specularStrength = 0.5f;
    vec3 cameraDir = normalize(cameraPosition - FragmentPosition);
    vec3 reflectedDir = reflect(-lightDir, norm);

    const float SHININESS = 256.0f;
    float spec = pow(max(dot(cameraDir, reflectedDir), 0.0f), SHININESS);
    vec3 specular = spec * specularStrength * lightColor;
    vec3 result = (diffuse + ambient + specular) * cubeColor;

    FragColor = vec4(result, 1.0f);
};
