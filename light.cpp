//
// Created by rowan on 22/12/2024.
//

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "light.h"
#include "utilities.h"

GLuint createDepthFramebuffer(int resolution, GLuint* depthTexture, LightType type) {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if (type == POINT_LIGHT) {
        // Create cube map texture for point light
        glGenTextures(1, depthTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, *depthTexture);

        // Set up the cube map faces for depth
        for (int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }

        // Cube map texture parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // Attach cube map texture as depth attachment
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *depthTexture, 0);
    } else {
        // Create single depth map for spotlight
        glGenTextures(1, depthTexture);
        glBindTexture(GL_TEXTURE_2D, *depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        // Texture parameters for spotlight depth map
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Attach depth map as depth attachment
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depthTexture, 0);
    }

    // Disable color buffer outputs
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 0;
    }

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbo;
}


// Constructor definition
Light::Light(glm::vec3 position, glm::vec3 colour, float intensity, int shadowMapSize, float lightRange, LightType type)
        : position(position), colour(colour), intensity(intensity), shadowMapSize(shadowMapSize), lightRange(lightRange), lightType(type) {
    shadowCubeMap = 0;
    shadowMap = 0;

    // Create the appropriate depth texture and framebuffer
    shadowFBO = createDepthFramebuffer(shadowMapSize, (lightType == POINT_LIGHT) ? &shadowCubeMap : &shadowMap, lightType);

    update();
}
// update() method definition
void Light::update() {
    calculateVPMatrices();
}

void Light::setDir(glm::vec3 dir){
    direction = dir;
    update();
}

// calculateVPMatrices() method definition
void Light::calculateVPMatrices() {
    if (lightType == POINT_LIGHT) {
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
    } else {
        direction = glm::normalize(direction);
        glm::vec3 anyUp = glm::normalize(glm::cross(direction, -direction));
        // For spotlight (single depth map), calculate projection and view matrix
        lightProjectionMatrix = glm::perspective(glm::radians(26.0f), 1.0f, 1.0f, lightRange);
        VPmatrix = lightProjectionMatrix * glm::lookAt(position, position+direction, glm::vec3(0.0, 0.0, 1.0));
    }
}