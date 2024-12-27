#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;

uniform mat4 MVPMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

out vec3 fragNormal;
out vec3 fragPosition;
out vec2 fragTexCoord;

void main() {
    gl_Position = MVPMatrix * vec4(vertexPosition, 1.0);
    fragPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    fragNormal = normalize(normalMatrix * vertexNormal);
    fragTexCoord = vertexUV;
}