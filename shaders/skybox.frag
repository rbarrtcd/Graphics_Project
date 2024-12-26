#version 330 core

in vec3 TexCoords;

layout (location = 0) out vec4 outputColour;
layout (location = 1) out vec3 worldPosition;
layout (location = 2) out vec3 frag_norm_vec;
layout (location = 3) out float emit;

uniform samplerCube skybox;

void main() {
    outputColour = vec4(texture(skybox, TexCoords).rgb, 1.0);
    //outputColour = vec4(0.3, 0.7, 0.5, 1.0);
    worldPosition = TexCoords;
    frag_norm_vec = vec3(0.0);
    emit = 1.0;

}
