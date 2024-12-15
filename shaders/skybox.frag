#version 330 core

in vec3 TexCoords;

out vec4 FragColor;

uniform samplerCube skybox;
float linearizeDepth(float depth, float near, float far) {
    float z = depth * 2.0 - 1.0;  // Transform depth to NDC (-1 to 1)
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
    float scale = linearizeDepth(texture(skybox, TexCoords).r, 1.0, 1240.0)/1240.0 ;
    FragColor = vec4(scale, scale, scale, 1);
}
