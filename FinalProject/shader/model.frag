#version 330 core

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoord;

uniform vec3 lightDir;  // Directional light direction (normalized)
uniform vec3 lightIntensity; // Light intensity (ambient + diffuse + specular)
uniform vec3 cameraPos; // Camera position for specular calculations
uniform sampler2D modelTexture;

float fogStart = 800.0;
float fogEnd = 1400.0;

out vec4 fragColor;

void main() {
    // fog
    float distance = length(fragPosition.xz - cameraPos.xz);
    float hight = length(fragPosition.y - cameraPos.y);
    float fogFactor = 1 - ((distance - fogStart) / (fogEnd - fogStart));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Normalize inputs
    vec3 norm = normalize(fragNormal);
    vec3 lightDirNormalized = normalize(lightDir) * vec3(-1);

    // Ambient lighting
    vec3 ambient = vec3(0.9);

    // Diffuse lighting
    float diff = max(dot(norm, lightDirNormalized), 0.0);
    vec3 diffuse = vec3(diff);

    // Specular lighting (Phong model)
    vec3 viewDir = normalize(cameraPos - fragPosition);
    vec3 reflectDir = reflect(-lightDirNormalized, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // Shininess factor = 32
    vec3 specular = vec3(spec);

    vec3 lighting = ambient + diffuse + specular;
    vec3 textureColor = texture(modelTexture, fragTexCoord).rgb;

    fragColor = vec4(lighting * normalize(lightIntensity) * textureColor, fogFactor);
}
