#version 330 core

struct Material {
    vec3 albedo;
    float roughness;
    float metallic;
    float ao;
}; 

struct DirLight {
    vec3 direction;
    vec3 color;
};

struct PointLight {
    vec3 position;
    vec3 color;
    
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;

    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;      
};

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

const float PI = 3.14159265359;
  
uniform vec3 viewPos;
uniform PointLight pLight[16];
uniform DirLight dLight;
uniform float pointLightCount;
  
uniform Material material;

uniform samplerCube irradianceMap;
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D armMap;


uniform bool textured;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

vec3 CalcPointLight(int i,vec3 N, vec3 V, vec3 F0,vec3 albedo, float roughness, float metallic){
    vec3 L = normalize(pLight[i].position - FragPos);
    vec3 H = normalize(V + L);

    float distance = length(pLight[i].position - FragPos);
    float attenuation = 1.0 / (pLight[i].constant + pLight[i].linear * distance + pLight[i].quadratic * (distance * distance));    

    vec3 radiance = pLight[i].color * attenuation;        

    float NDF = DistributionGGX(N, H, roughness);        
    float G = GeometrySmith(N, V, L, roughness);      
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);       
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;  
        
    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);                
    return (kD * albedo / PI + specular) * radiance * NdotL; 
}

vec3 CalcDirLight(vec3 N, vec3 V,vec3 F0,vec3 albedo, float roughness, float metallic)
{
    vec3 L = normalize(-dLight.direction);
    vec3 H = normalize(V + L);

    vec3 radiance = max(dot(N, L), 0.0) * dLight.color;

    // float shadow = ShadowCalculation(FragPosLightSpace, N, L);

    float NDF = DistributionGGX(N, H, roughness);        
    float G = GeometrySmith(N, V, L, roughness);      
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);       
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;  
        
    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);                
    return (kD * albedo / PI + specular) * radiance * NdotL; 
}

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(FragPos);
    vec3 Q2  = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
// {
//     vec3 lightDir = normalize(light.position - fragPos);

//     float diff = max(dot(normal, lightDir), 0.0);

//     vec3 halfwayDir = normalize(lightDir + viewDir);
//     float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

//     float distance = length(light.position - fragPos);
//     float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

//     float theta = dot(lightDir, normalize(-light.direction));
//     float epsilon = light.cutOff - light.outerCutOff;
//     float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

//     vec3 ambient = light.ambient * material.diffuse;
//     vec3 diffuse = light.diffuse * diff * material.diffuse;
//     vec3 specular = light.specular * spec * material.specular;

//     ambient *= attenuation * intensity;
//     diffuse *= attenuation * intensity;
//     specular *= attenuation * intensity;

//     return (ambient + diffuse + specular);
// }



void main(){
    vec3 albedo     = material.albedo;
    float ao = material.ao;        
    float roughness = material.roughness;  
    float metallic = material.metallic;   
    vec3 N;

    if(textured){
        albedo     *= pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
        vec3 arm = texture(armMap, TexCoords).rgb;
        ao *= arm.r;        
        roughness *= arm.g;  
        metallic *= arm.b;   
        N = getNormalFromMap();
    }
    else{
        N = normalize(Normal);
    }

    vec3 V = normalize(viewPos - FragPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = CalcDirLight(N,V,F0,albedo,roughness,metallic);
    for(int i = 0; i < pointLightCount; i++){
        Lo += CalcPointLight(i,N,V, F0,albedo,roughness,metallic);
    }

    vec3 kS = fresnelSchlick(max(dot(N, V), 0.0), F0); 
    vec3 kD = 1.0 - kS;
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * albedo;
    vec3 ambient    = (kD * diffuse) * ao; 

    // vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    FragColor = vec4(color, 1.0);
}