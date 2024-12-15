#version 330 core

layout(location = 0) in vec3 aPosition;

uniform mat4 view;
uniform mat4 projection;

out vec3 TexCoords;

void main() {
    // Remove translation from the view matrix to keep the skybox stationary

    TexCoords = aPosition;

    gl_Position = projection * view * vec4(aPosition, 1.0);
}
