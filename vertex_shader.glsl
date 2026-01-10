#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec2 aTexPos;

out vec4 ourColor;
out vec2 ourTexPos;

uniform mat4 transform;

void main()
{
   gl_Position = transform * vec4(aPos, 1.0);
   ourColor = vec4(aCol, 1.0);
   ourTexPos = aTexPos;
};
