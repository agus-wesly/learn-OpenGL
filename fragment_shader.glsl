#version 330 core

in vec4 ourColor;
in vec4 pos;
out vec4 FragColor;

void main()
{
    FragColor = pos;
};
