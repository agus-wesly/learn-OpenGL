#version 330 core

uniform vec3 lightColor;
uniform vec3 cubeColor;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

in vec3 Normal;
in vec3 fragmentPosition;

out vec4 FragColor;

void main()
{
    float ambientStrength = 0.4f;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPosition - fragmentPosition);
    float diffuseStrength = max(dot(lightDir, norm), 0.0f);
    vec3 diffuse = diffuseStrength * lightColor;

    const float specularStrength = 0.5f;
    vec3 cameraDir = normalize(cameraPosition - fragmentPosition);
    vec3 reflectedDir = reflect(-lightDir, norm);
    const float SHININESS = 2.0f;
    float spec = pow(max(dot(cameraDir, reflectedDir), 0.0f), SHININESS);
    vec3 specular = spec * specularStrength * lightColor;

    vec3 result = (diffuse + ambient + specular) * cubeColor;
    FragColor = vec4(result, 1.0f);
};
