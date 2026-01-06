#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 col;
out vec4 ourColor;
out vec4 pos;
uniform float xOffset;

void main()
{
   gl_Position = vec4(aPos.x + xOffset, aPos.y, aPos.z, 1.0);
   pos = gl_Position;
};
