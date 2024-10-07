#version 330 core
out vec4 FragColor;

in vec3 color;
in vec3 normal;
in vec3 crntPos;

uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;

void main()
{
	float ambient = 0.2f;

	vec3 n = normalize(normal);
	vec3 lightDirection = normalize(lightPos - crntPos);

	float diffuse = max(dot(n, lightDirection), 0.0f);

	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8);
	float specular = specAmount * specularLight;

	FragColor = vec4(color,1.0f) * lightColor * (diffuse + ambient + specular);
}