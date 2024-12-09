#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec4 joints;
layout(location = 4) in vec4 weights;


out vec3 worldPosition;
out vec3 worldNormal;

uniform mat4 MVP;
uniform mat4 jointMatrix[25];

void main() {
    vec4 newWorldPosition = vec4(0.0);
    vec3 blendedNormal = vec3(0.0);

    mat4 skinMat =
            weights.x * jointMatrix[int(joints.x)] +
            weights.y * jointMatrix[int(joints.y)] +
            weights.z * jointMatrix[int(joints.z)] +
            weights.w * jointMatrix[int(joints.w)];

    blendedNormal =
        weights.x * mat3(jointMatrix[int(joints.x)]) * vertexNormal
        + weights.y * mat3(jointMatrix[int(joints.y)]) * vertexNormal
        + weights.z * mat3(jointMatrix[int(joints.z)]) * vertexNormal
        + weights.w * mat3(jointMatrix[int(joints.w)]) * vertexNormal;

    newWorldPosition = skinMat * vec4(vertexPosition, 1.0);
    gl_Position = MVP * newWorldPosition;

    worldPosition = newWorldPosition.xyz;
    worldNormal = normalize(blendedNormal);
}
