#version 330 core
layout (location = 0) in vec3 vertexPosition;

uniform mat4 lightSpaceMatrix;
uniform mat4 Model;

void main() {
    gl_Position = lightSpaceMatrix * Model * vec4(vertexPosition, 1);
}