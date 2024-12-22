#ifndef LAB4_LIGHT_H
#define LAB4_LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>



class Light {
public:
    glm::vec3 position;
    glm::vec3 colour;
    float intensity;
    float lightRange;
    int shadowMapSize;

    GLuint shadowFBO;
    GLuint shadowCubeMap;

    glm::mat4 lightProjectionMatrix;

    glm::mat4 VPmatrices[6];

    Light(glm::vec3 position, glm::vec3 colour, float intensity, int shadowMapSize, float lightRange);

    void update();

    void calculateVPMatrices();

};

#endif // LAB4_LIGHT_H