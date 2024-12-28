#version 330 core
layout(location = 0) in vec3 vertexPosition;

uniform mat4 modelMatrix;
out vec4 fragPos;
void main() {
    gl_Position = modelMatrix * vec4(vertexPosition, 1.0);
}
