#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec3 vertexNormal;
layout(location = 4) in vec4 weights;  // Weights for each joint influence
layout(location = 5) in vec4 indices;  // Indices of the joints

uniform mat4 jointMatrix[100]; // Joint transformation matrices

out vec4 fragColor;
out vec3 fragNormal;
out vec2 fragUV;
out vec3 fragPosition;

uniform mat4 modelMatrix;
uniform mat4 MVP;

void main() {
    // Skinning transformation
    mat4 skinMat = mat4(0.0);
    float boneSum = weights.x + weights.y + weights.z + weights.w;
    // Accumulate joint transformations using weights
    skinMat += weights.x * jointMatrix[int(indices.x)];
    skinMat += weights.y * jointMatrix[int(indices.y)];
    skinMat += weights.z * jointMatrix[int(indices.z)];
    skinMat += weights.w * jointMatrix[int(indices.w)];

    // Transform position by skinning matrix
    vec4 skinnedPosition =     (skinMat / boneSum) * vec4(vertexPosition, 1.0);

    // Calculate the normal matrix for the skinning matrix
    mat3 skinNormalMatrix = mat3(transpose(inverse(skinMat)));
    vec3 skinnedNormal = normalize(skinNormalMatrix * vertexNormal);

    // Transform position to world space
    vec4 worldPos = modelMatrix * skinnedPosition;
    fragPosition = worldPos.xyz;

    // Pass the color and UV to the fragment shader
    fragColor = vec4(vertexColor, 1.0);
    fragUV = vertexUV;

    // Transform skinned normal to world space
    mat3 modelNormalMatrix = mat3(transpose(inverse(modelMatrix)));
    fragNormal = normalize(modelNormalMatrix * skinnedNormal);

    // Transform the vertex position to clip space
    gl_Position = MVP * skinnedPosition;
}
