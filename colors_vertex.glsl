#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 Normal;
out vec3 FragmentPosition;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;
uniform mat3 normalMatrix;

void main()
{
   FragmentPosition = vec3(model * vec4(aPos, 1.0f));
   Normal = normalMatrix * aNormal;
   TexCoords = aTexCoords;

   gl_Position = perspective * view * vec4(FragmentPosition, 1.0f);
};
