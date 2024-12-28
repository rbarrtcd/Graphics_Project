#version 330 core
layout(location = 0) in vec3 vertexPosition;

uniform mat4 modelMatrix;
uniform mat4 MVP;
out vec4 FragPos;
void main() {
    FragPos = modelMatrix * vec4(vertexPosition, 1.0);
    gl_Position = MVP * vec4(vertexPosition, 1.0);
}
