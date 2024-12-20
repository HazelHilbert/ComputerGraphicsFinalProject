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

out vec3 finalColor;

void main()
{
    // shadow mapping
    vec4 lightSpacePosition = lightSpaceMatrixRender * vec4(fragPos, 1.0);
    vec3 lightSpaceNDC = lightSpacePosition.xyz / lightSpacePosition.w;
    vec3 lightSpace0to1 = (lightSpaceNDC * 0.5) + 0.5;
    vec2 shadowUV = lightSpace0to1.xy;
    //shadowUV.y = 1.0 - shadowUV.y; // Attempt to flip
    float depth = lightSpace0to1.z;
    float existingDepth = texture(depthTextureSampler, shadowUV).r;
    float shadow = (depth >= existingDepth + 1e-3) ? 0.2 : 1.0;
    if (any(lessThan(lightSpace0to1, vec3(0,0,0))) || any(greaterThan(lightSpace0to1, vec3(1,1,1)))) {
        //shadow = 1.0;
    }

    // lighting, tone mapping, gamma correction
    float theta = max(dot(normal, -lightDirection), 0.0);
    vec3 texureColor = texture(textureSampler, uv).rgb;
    vec3 lambertianColor =  shadow * theta * normalize(lightIntensity) * texureColor;
    vec3 toneColor = lambertianColor / (1.0 + lambertianColor);
    vec3 sRGB = pow(toneColor, vec3(1.0 / gamma));
    finalColor = lambertianColor;

    // both depth and existingDepth is smaller than 0.1 but bigger than 0. depth is always > then existingDepth
    //if (shadow == 1){
    //finalColor = vec3(existingDepth,0,0);
    //finalColor = vec3(uv.x, uv.y, 0);
    //finalColor = vec3(shadowUV.x, shadowUV.y, 0);
    //}
}
