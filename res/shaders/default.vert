#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in vec2 aTex;


out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 camMatrix;
uniform mat4 model;


void main()
{

	FragPos = vec3(model *vec4(aPos, 1.0f));

	Normal = mat3(transpose(inverse(model))) * aNormal;

	TexCoords = mat2(0.0, -1.0, 1.0, 0.0) * aTex;
	
	gl_Position = camMatrix * vec4(FragPos, 1.0);
}