#version 330 core

out vec4 FragColor;

uniform mat4 invCameraMatrix;   // Inverse of the combined view-projection matrix
uniform vec3 cameraPosition;    // Camera position in world space

uniform vec2 iResolution;
uniform float iTime;

uniform vec3 sunDirection;
uniform vec3 sunColor;

uniform sampler2D uBlueNoise;
uniform sampler2D uNoise;
uniform int uFrame;

uniform float baseMarchSize;
uniform float lightMarchSize;
uniform float absorptionCoEff;

#define MAX_STEPS 100
#define MAX_STEPS_LIGHTS 6

// Exponential attenuation
float BeersLaw(float dist, float absorption) {
    return exp(-dist * absorption);
}

// Signed Distance Functions
float sdSphere(vec3 p, float radius) {
    return length(p) - radius;
}

float sdTorus(vec3 p, vec2 t) {
    vec2 q = vec2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

// Scene definition
float scene(vec3 p) {
    return -sdSphere(p, 0.25);  // Negative distance for volume inside the torus
    // return -sdTorus(p, vec2(0.3,0.2)); 
    // return -sdBox(p, vec3(0.5));
}

float animatedNoise(vec3 p) {
    // Simple sine-based noise for animation
    return sin(p.x * 10.0 + p.y * 10.0 + iTime * 2.0) * 0.05;
}

// float scene(vec3 p) {
//     float baseShape = -sdBox(p, vec3(0.3)); // Base distance field (box in this case)
//     float noise = animatedNoise(p);        // Add animated noise
//     return baseShape + noise;              // Combine base shape and noise
// }

// Marching light rays
float lightmarch(vec3 position) {
    float totalDensity = 0.0;

    for (int step = 0; step < MAX_STEPS_LIGHTS; step++) {
        position += sunDirection * lightMarchSize;
        float lightSample = scene(position);
        totalDensity += lightSample;
    }

    float transmittance = BeersLaw(totalDensity, absorptionCoEff);
    return transmittance;
}

// Raymarching for clouds or volumes
float raymarch(vec3 ro, vec3 rd, float offset) {
    float depth = 0.0;
    depth += baseMarchSize * offset; // Start marching with offset
    vec3 p = ro + depth * rd;

    float totalTransmittance = 1.0;
    float lightEnergy = 0.0;

    for (int i = 0; i < MAX_STEPS; i++) {
        float density = scene(p);

        // Accumulate light only for density > 0
        if (density > 0.0) {
            float transmittance = lightmarch(p);
            float luminance = density;

            totalTransmittance *= transmittance;
            lightEnergy += totalTransmittance * luminance;
        }

        // Adapt march size based on depth to improve stability
        float adaptiveMarchSize = max(baseMarchSize * depth, 0.001);
        depth += adaptiveMarchSize;
        p = ro + depth * rd;  // Advance along the ray
    }

    return clamp(lightEnergy, 0.0, 1.0);
}

void main() {
    // Compute ray origin and direction
    vec4 ndc = vec4((gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0, -1.0, 1.0); // NDC coordinates
    vec4 worldPos = invCameraMatrix * ndc;
    worldPos /= worldPos.w; // Perspective divide

    vec3 ro = cameraPosition;              // Ray origin (camera position)
    vec3 rd = normalize(worldPos.xyz - ro); // Ray direction (from camera to world position)

    vec3 color = vec3(0);

    // color = vec3(0.7,0.7,0.90);

    // Use blue noise to add randomness for dithering
    // Average several samples from the blue noise texture for smoothing
    vec2 uv = gl_FragCoord.xy / 1024.0;
    float blueNoise = 0.0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            blueNoise += texture2D(uBlueNoise, uv + vec2(i, j) / 1024.0).r;
        }
    }
    blueNoise /= 9.0; // Average over a 3x3 grid

    float offset = fract(blueNoise + float(uFrame % 32) / sqrt(0.5));
    // Perform raymarching
    float res = raymarch(ro, rd, offset);
    color += sunColor * res; 

    color = pow(color, vec3(1.0 / 2.2));

    // Output the final color
    FragColor = vec4(color, 1.0);
}
