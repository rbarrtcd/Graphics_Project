//
// Created by rowan on 23/12/2024.
//
#ifndef DEFERRED_SHADER_H
#define DEFERRED_SHADER_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "light.h"


class DeferredShader {
public:
    // Constructor and Destructor
    DeferredShader(GLuint programID);
    ~DeferredShader();

    // Initialization and cleanup
    void initialize();
    void cleanup();

    // Rendering method
    void render(GLuint gBuffer, GLuint gColour, GLuint gPosition, GLuint gNormal, GLuint gEmit, std::vector<Light*> lights);

private:
    // Vertex buffer data for a fullscreen quad
    GLfloat vertex_buffer_data[16] = {
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f,  1.0f,  1.0f, 1.0f,
    };

    GLuint index_buffer_data[6] = {
            0, 1, 2,
            2, 1, 3,
    };

    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;

    GLuint programID;
    GLuint gPositionID;
    GLuint gNormalID;
    GLuint gAlbedoSpecID;
    GLuint gEmitID;

    GLuint depthMapArrayID;
    GLuint lightPositionArrayID;
    GLuint lightColorArrayID;
    GLuint lightIntensityArrayID;
    GLuint lightRangeArrayID;
    GLuint numLightsID;
};

#endif // DEFERRED_SHADER_H
