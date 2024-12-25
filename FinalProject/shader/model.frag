#version 330 core

in vec3 fragNormal;
in vec2 fragTexCoord;

uniform vec3 lightPos;
uniform vec3 lightIntensity;
uniform sampler2D modelTexture;

out vec4 fragColor;

void main() {
    vec3 norm = normalize(fragNormal);
    float diff = max(dot(norm, normalize(lightPos)), 0.0);

    vec3 color = texture(modelTexture, fragTexCoord).rgb; // Sample texture normalize(lightIntensity) * diff *
    fragColor = vec4(color, 1.0);
}
