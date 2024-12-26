#version 330 core

layout(location = 0) in vec3 aPosition;

uniform mat4 view; // Should be passed with translation removed
uniform mat4 projection;

out vec3 TexCoords;

void main() {
    TexCoords = aPosition;

    // Apply projection and view transformation
    vec4 pos = projection * view * vec4(aPosition, 1.0);

    // Ensure the skybox vertices have a far clipping depth
    gl_Position = pos; // Force depth to the far plane
}
