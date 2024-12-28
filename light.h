#ifndef LAB4_LIGHT_H
#define LAB4_LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

enum LightType {
    POINT_LIGHT, // Cube map (6 faces)
    SPOT_LIGHT   // Single depth map (1 face)
};

class Light {
public:
    glm::vec3 position;
    glm::vec3 colour;
    glm::vec3 direction;
    float intensity;
    float lightRange;
    int shadowMapSize;

    GLuint shadowFBO;
    GLuint shadowCubeMap;  // Depth cube map for point light
    GLuint shadowMap;      // Depth map for spotlight (single texture)

    glm::mat4 lightProjectionMatrix;
    glm::mat4 VPmatrices[6];  // For point lights (cube map)
    glm::mat4 VPmatrix;      // For spotlights (single depth map)

    Light(glm::vec3 position, glm::vec3 colour, float intensity, int shadowMapSize, float lightRange, LightType type);

    void setDir(glm::vec3 dir);
    void update();
    void calculateVPMatrices();


    LightType lightType;  // Store the type of light (point or spotlight)

};

#endif // LAB4_LIGHT_H
