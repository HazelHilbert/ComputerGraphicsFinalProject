#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec3 vertexNormal;

out vec2 uv;
out vec3 fragPos;
out vec3 normal;

// Matrix for vertex transformation
uniform mat4 MVP;
uniform mat4 Model;

void main() {
    gl_Position =  MVP * vec4(vertexPosition, 1);

    uv = vertexUV;

    // Calculate world position of the fragment
    fragPos = vec3(Model * vec4(vertexPosition, 1.0));
    //fragPos = vertexPosition;

    // Transform the normal vector to world space
    normal = mat3(transpose(inverse(Model))) * vertexNormal;
    //normal = vertexNormal;
}
