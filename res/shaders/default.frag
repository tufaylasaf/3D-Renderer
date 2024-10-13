#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

struct Light {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 FragColor;

in vec3 crntPos;
in vec3 Normal;
in vec2 texCoord;

uniform sampler2D diffuse0;
uniform sampler2D specular0;

uniform Material material;
uniform Light light; 
uniform vec3 camPos;



void main()
{
	//ambient
    vec3 ambient = material.ambient * light.ambient;

	//diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - crntPos); 
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = (diff * material.diffuse) * light.diffuse;

	//specular
	vec3 viewDir = normalize(camPos - crntPos);
	vec3 reflectDir = reflect(-lightDir, norm); 
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = (spec * material.specular) * light.specular;  

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result,1.0);
}