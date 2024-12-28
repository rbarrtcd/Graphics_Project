#version 330 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec4 weights;  // Weights for each joint influence
layout(location = 2) in vec4 indices;  // Indices of the joints

uniform mat4 modelMatrix;
uniform mat4 jointMatrix[100]; // Joint transformation matrices
uniform mat4 MVP;

out vec4 FragPos;

void main() {
    mat4 skinMat = mat4(0.0);
    float boneSum = weights.x + weights.y + weights.z + weights.w;
    // Accumulate joint transformations using weights
    skinMat += weights.x * jointMatrix[int(indices.x)];
    skinMat += weights.y * jointMatrix[int(indices.y)];
    skinMat += weights.z * jointMatrix[int(indices.z)];
    skinMat += weights.w * jointMatrix[int(indices.w)];

    // Transform position by skinning matrix
    vec4 skinnedPosition = (skinMat / boneSum) * vec4(vertexPosition, 1.0);
    FragPos = modelMatrix * vec4(vec3(skinnedPosition), 1.0);
    gl_Position = MVP * vec4(vec3(skinnedPosition), 1.0);
}
