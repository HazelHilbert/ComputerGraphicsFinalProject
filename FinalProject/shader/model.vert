#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;

uniform mat4 MVPMatrix;

out vec3 fragNormal;
out vec2 fragTexCoord;

void main() {
    gl_Position = MVPMatrix * vec4(vertexPosition, 1.0);
    fragNormal = vertexNormal;
    fragTexCoord = vertexTexCoord;
}