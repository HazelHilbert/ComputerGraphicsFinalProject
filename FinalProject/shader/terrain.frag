#version 330 core

in vec2 uv;
in vec3 fragPos;
in vec3 normal;

uniform sampler2D textureSampler;
uniform sampler2D depthTextureSampler;
uniform vec3 lightDirection;
uniform vec3 lightIntensity;

uniform mat4 lightSpaceMatrixRender;

const float gamma = 2.2;
const float bias = 1e-3;
const int PCF_SIZE = 3;

out vec3 finalColor;

void main()
{
    // shadow mapping
    vec4 lightSpacePosition = lightSpaceMatrixRender * vec4(fragPos, 1.0);
    vec3 lightSpaceNDC = lightSpacePosition.xyz / lightSpacePosition.w;
    vec3 lightSpace0to1 = (lightSpaceNDC * 0.5) + 0.5;
    vec2 shadowUV = lightSpace0to1.xy;
    float depth = lightSpace0to1.z;

    // Percentage-Closer Filtering (PCF)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(2048,1563); // TODO: pass depth map size
    for(int x = -PCF_SIZE; x <= PCF_SIZE; ++x) {
        for(int y = -PCF_SIZE; y <= PCF_SIZE; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            float existingDepth = texture(depthTextureSampler, shadowUV + offset).r;
            if(depth < existingDepth + bias) shadow += 1.0;
        }
    }
    int totalSamples = (2 * PCF_SIZE + 1) * (2 * PCF_SIZE + 1);
    shadow /= float(totalSamples);

    if(any(lessThan(lightSpace0to1, vec3(0.0))) || any(greaterThan(lightSpace0to1, vec3(1.0)))) shadow = 1.0;

    // lighting, tone mapping, gamma correction
    float theta = max(dot(normal, -lightDirection), 0.0);
    vec3 texureColor = texture(textureSampler, uv).rgb;
    vec3 lambertianColor = theta * normalize(lightIntensity) * texureColor;
    vec3 toneColor = lambertianColor / (1.0 + lambertianColor);
    vec3 sRGB = pow(toneColor, vec3(1.0 / gamma));
    finalColor = lambertianColor;

   // finalColor = vec3(shadow);
}
