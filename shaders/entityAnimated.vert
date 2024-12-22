#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec3 vertexNormal;
layout(location = 4) in vec4 weights;  // Weights for each joint influence
layout(location = 5) in vec4 indices;  // Indices of the joints

uniform mat4 jointMatrix[100];

out vec4 fragColor;
out vec3 fragNormal;
out vec2 fragUV;
out vec3 fragPosition;

uniform mat4 modelMatrix;
uniform mat4 MVP;

void main() {
    vec4 newWorldPosition = vec4(0.0);
    vec3 blendedNormal = vec3(0.0);

    mat4 skinMat = mat4(1.0);

    // Accumulate joint transformations only for valid indices
    if (indices.x >= 0.0 && weights.x >= 0.0) skinMat += weights.x * jointMatrix[int(indices.x)];
    if (indices.y >= 0.0 && weights.y >= 0.0) skinMat += weights.y * jointMatrix[int(indices.y)];
    if (indices.z >= 0.0 && weights.z >= 0.0) skinMat += weights.z * jointMatrix[int(indices.z)];
    if (indices.w >= 0.0 && weights.w >= 0.0) skinMat += weights.w * jointMatrix[int(indices.w)];

    // Transform vertex position and normal using the skinning matrix
    newWorldPosition = skinMat * vec4(vertexPosition, 1.0);
    //newWorldPosition = vec4(vertexPosition, 1.0);


    // Calculate the normal matrix for skinning
    mat3 normalMatrix = mat3(transpose(inverse(skinMat)));
    blendedNormal = normalize(normalMatrix * vertexNormal);

    // Calculate world-space position
    vec4 worldPos = modelMatrix * newWorldPosition;
    fragPosition = worldPos.xyz;

    // Pass the color and normal to the fragment shader
    fragColor = vec4(vertexColor, 1.0);
    fragNormal = normalize(blendedNormal);  // World space normal
    fragUV = vertexUV;

    // Transform the vertex position to clip space
    gl_Position = MVP * newWorldPosition;
}
