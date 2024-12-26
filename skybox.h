// Skybox.h

#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>
#include <vector>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
class Skybox {
public:
    GLuint VAO;
    GLuint VBO;
    int skyboxIndex;
    std::vector<GLuint> textureIDs;
    GLuint shaderID;

    Skybox();

    void Skybox::initialise(GLuint shaderID);
    void render(const glm::mat4& view, const glm::mat4& projection);
    GLuint Skybox::addSkybox(const std::vector<std::string>& paths);


private:
    std::vector<float> skyboxVertices;

    GLuint Skybox::loadCubemap(const std::vector<std::string>& paths);
    void setupSkybox();
};

#endif
