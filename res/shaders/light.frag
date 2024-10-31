#version 330 core

out vec4 FragColor;

struct Material {
    vec3 albedo;
    float roughness;
    float metallic;
    float ao;
}; 


uniform Material material;

void main()
{
	FragColor = vec4(material.albedo,1.0);
}