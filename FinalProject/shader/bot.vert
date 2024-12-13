#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec4 vertexJoint;
layout(location = 4) in vec4 vertexWeight;

// Output data, to be interpolated for each fragment
out vec3 worldPosition;
out vec3 worldNormal;

uniform mat4 MVP;
uniform mat4 jointMatrices[25];

void main() {
    mat4 skinMatrix =   vertexWeight.x * jointMatrices[int(vertexJoint.x)] +
                        vertexWeight.y * jointMatrices[int(vertexJoint.y)] +
                        vertexWeight.z * jointMatrices[int(vertexJoint.z)] +
                        vertexWeight.w * jointMatrices[int(vertexJoint.w)];

    vec4 skinnedPosition = skinMatrix * vec4(vertexPosition, 1.0);

    worldPosition = skinnedPosition.xyz;
    worldNormal = normalize(mat3(skinMatrix) * vertexNormal);

    gl_Position = MVP * skinnedPosition;
}
