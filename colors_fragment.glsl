#version 330 core

uniform vec3 lightColor;
uniform vec3 cubeColor;
uniform vec3 lightPosition;

in vec3 Normal;
in vec3 fragmentPosition;

out vec4 FragColor;

void main()
{
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPosition - fragmentPosition);
    float diffuseStrength = max(dot(lightDir, norm), 0.0f);
    vec3 diffuse = diffuseStrength * lightColor;

    vec3 result = (diffuse + ambient) * cubeColor;
    FragColor = vec4(result, 1.0f);
};
