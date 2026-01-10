#version 330 core

in vec4 ourColor;
in vec2 ourTexPos;

out vec4 FragColor;

uniform sampler2D ourTexture1;

void main()
{
    FragColor = texture(ourTexture1, ourTexPos);
};
