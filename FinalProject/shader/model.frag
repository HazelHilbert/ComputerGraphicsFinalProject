#version 330 core

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoord;

uniform vec3 lightDir;  // Directional light direction (normalized)
uniform vec3 lightIntensity; // Light intensity (ambient + diffuse + specular)
uniform vec3 cameraPos; // Camera position for specular calculations
uniform sampler2D modelTexture;

out vec4 fragColor;

void main() {
    // Normalize inputs
    vec3 norm = normalize(fragNormal);
    vec3 lightDirNormalized = normalize(lightDir);

    // Ambient lighting
    vec3 ambient = vec3(0.9);

    // Diffuse lighting
    float diff = max(dot(norm, lightDirNormalized), 0.0);
    vec3 diffuse = diff * normalize(lightIntensity);

    // Specular lighting (Phong model)
    vec3 viewDir = normalize(cameraPos - fragPosition);
    vec3 reflectDir = reflect(-lightDirNormalized, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // Shininess factor = 32
    vec3 specular = spec * lightIntensity;

    // Combine results
    vec3 lighting = ambient + diffuse + specular; //ambient; //t + diffuse + specular;
    vec3 textureColor = texture(modelTexture, fragTexCoord).rgb;
    vec3 finalColor = lighting * textureColor;

    fragColor = vec4(finalColor, 1.0);
}
