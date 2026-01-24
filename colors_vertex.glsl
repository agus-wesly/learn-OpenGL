#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;


out vec2 ourTexPos;
out vec3 Normal;
out vec3 fragmentPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;
uniform mat3 normalMatrix;

void main()
{
   fragmentPosition = vec3(model * vec4(aPos, 1.0f));
   Normal = normalMatrix * aNormal;
   gl_Position = perspective * view * vec4(fragmentPosition, 1.0f);
};
