#version 330 core

in vec4 ourColor;
in vec2 ourTexPos;

out vec4 FragColor;

uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;

void main()
{
    FragColor = mix(texture(ourTexture1, ourTexPos), texture(ourTexture2, vec2(ourTexPos.x, ourTexPos.y)), 0.5);
};
