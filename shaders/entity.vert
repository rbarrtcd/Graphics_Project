#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec3 vertexNormal;

out vec4 fragColor;
out vec3 fragNormal;
out vec2 fragUV;
out vec3 fragPosition;

uniform mat4 modelMatrix;
uniform mat4 MVP;


void main() {

    // Calculate world-space position
    vec4 worldPos = modelMatrix * vec4(vertexPosition, 1.0);
    fragPosition = worldPos.xyz;

    // Pass the color and normal to the fragment shader
    fragColor = vec4(vertexColor, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    fragNormal = normalize(normalMatrix * vertexNormal);
    fragUV = vertexUV;

    // Transform the vertex position to clip space
    gl_Position = MVP * vec4(vertexPosition, 1.0);
}