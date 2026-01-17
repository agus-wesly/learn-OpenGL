#version 330 core

uniform vec3 light;
uniform vec3 color;

out vec4 FragColor;

void main()
{
    FragColor = vec4(light * color, 1.0f);
};
