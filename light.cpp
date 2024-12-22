//
// Created by rowan on 22/12/2024.
//

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "light.h"
#include "utilities.h"

GLuint createDepthCubeMapFramebuffer(int resolution, GLuint* depthCubeMap) {
    GLuint fbo;

    // Generate and bind the framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Generate the cube map texture
    glGenTextures(1, depthCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *depthCubeMap);


    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X , 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X , 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y , 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y , 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z , 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z , 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);


    // Set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Attach the cube map texture to the framebuffer as the depth attachment
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *depthCubeMap, 0);

    // Disable color buffer outputs since this is a depth-only framebuffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 0;
    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fbo;
}


// Constructor definition
Light::Light(glm::vec3 position, glm::vec3 colour, float intensity, int shadowMapSize, float lightRange) {
    this->position = position;
    this->colour = colour;
    this->intensity = intensity;
    this->shadowMapSize = shadowMapSize;
    this->lightRange = lightRange;
    this->shadowCubeMap = 0;

    // Create the depth cube map and framebuffer
    shadowFBO = createDepthCubeMapFramebuffer(shadowMapSize, &shadowCubeMap);

    update();
}

// update() method definition
void Light::update() {
    calculateVPMatrices();
}

// calculateVPMatrices() method definition
void Light::calculateVPMatrices() {
    lightProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, lightRange);
    glm::vec3 directions[6] = {
            glm::vec3(1.0f, 0.0f, 0.0f),   // +X
            glm::vec3(-1.0f, 0.0f, 0.0f),  // -X
            glm::vec3(0.0f, 1.0f, 0.0f),   // +Y
            glm::vec3(0.0f, -1.0f, 0.0f),  // -Y
            glm::vec3(0.0f, 0.0f, 1.0f),   // +Z
            glm::vec3(0.0f, 0.0f, -1.0f)   // -Z
    };

    glm::vec3 upVectors[6] = {
            glm::vec3(0.0f, -1.0f, 0.0f),  // +X
            glm::vec3(0.0f, -1.0f, 0.0f),  // -X
            glm::vec3(0.0f, 0.0f, 1.0f),   // +Y
            glm::vec3(0.0f, 0.0f, -1.0f),  // -Y
            glm::vec3(0.0f, -1.0f, 0.0f),  // +Z
            glm::vec3(0.0f, -1.0f, 0.0f)   // -Z
    };

    for (int i = 0; i < 6; ++i) {
        VPmatrices[i] = lightProjectionMatrix * glm::lookAt(position, position + directions[i], upVectors[i]);
    }
}

