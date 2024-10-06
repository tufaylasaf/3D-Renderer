#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;


out vec3 color;
out vec2 texCoord;
uniform float scale;

uniform mat4 camMatrix;


void main()
{

	gl_Position = camMatrix * vec4(aPos, 1.0);

	color = aColor;
}