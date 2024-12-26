#include "DeferredShader.h"
#include "Light.h"  // Include the header for Light class if needed
#include "glm/gtc/type_ptr.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>


DeferredShader::DeferredShader(GLuint programID) : programID(programID) {
    // Constructor implementation
    vertexArrayID = 0;
    vertexBufferID = 0;
    indexBufferID = 0;
}

DeferredShader::~DeferredShader() {
    // Cleanup resources if necessary
    cleanup();
}

void DeferredShader::initialize() {
    // Generate and bind the vertex array object
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // Generate and bind the vertex buffer object
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

    // Generate and bind the index buffer object
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

    // Get uniform locations for G-buffer textures
    gPositionID = glGetUniformLocation(programID, "gPosition");
    gNormalID = glGetUniformLocation(programID, "gNormal");
    gAlbedoSpecID = glGetUniformLocation(programID, "gColour");
    gEmitID = glGetUniformLocation(programID, "gEmit");

    // Get uniform locations for light properties arrays
    depthMapArrayID = glGetUniformLocation(programID, "depthMaps");
    lightPositionArrayID = glGetUniformLocation(programID, "lPosition");
    lightColorArrayID = glGetUniformLocation(programID, "lColour");
    lightIntensityArrayID = glGetUniformLocation(programID, "lIntensity");
    lightRangeArrayID = glGetUniformLocation(programID, "lRange");
    numLightsID = glGetUniformLocation(programID, "numLights");
}

void DeferredShader::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
}

void DeferredShader::render(GLuint gBuffer, GLuint gColour, GLuint gPosition, GLuint gNormal, GLuint gEmit, std::vector<Light*> lights) {
    // Use the shader program
    glUseProgram(programID);

    // Bind G-buffer textures to texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gColour);
    glUniform1i(gAlbedoSpecID, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glUniform1i(gPositionID, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glUniform1i(gNormalID, 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gEmit);
    glUniform1i(gEmitID, 3);

    // Prepare light data for uploading
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    std::vector<float> lightIntensities;
    std::vector<float> lightRanges;
    std::vector<GLint> cubeMapIndices;

    for (size_t i = 0; i < lights.size(); ++i) {
        Light* light = lights[i];
        glActiveTexture(GL_TEXTURE4 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, light->shadowCubeMap);

        cubeMapIndices.push_back(4 + i);

        // Collect light properties
        lightPositions.push_back(light->position);
        lightColors.push_back(light->colour);
        lightIntensities.push_back(light->intensity);
        lightRanges.push_back(light->lightRange);
    }

    // Upload light data to the shader
    glUniform3fv(lightPositionArrayID, lightPositions.size(), glm::value_ptr(lightPositions[0]));
    glUniform3fv(lightColorArrayID, lightColors.size(), glm::value_ptr(lightColors[0]));
    glUniform1fv(lightIntensityArrayID, lightIntensities.size(), lightIntensities.data());
    glUniform1fv(lightRangeArrayID, lightRanges.size(), lightRanges.data());
    glUniform1i(numLightsID, static_cast<int>(lights.size()));
    glUniform1iv(depthMapArrayID, cubeMapIndices.size(), cubeMapIndices.data()); // Ensure the shader knows the correct texture unit

    // Bind the vertex array and set up vertex attributes
    glBindVertexArray(vertexArrayID);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

    // Render the fullscreen quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
