//
// Created by rowan on 22/12/2024.
//
#include "skybox.h"
#include "glm/gtc/type_ptr.hpp"
#include <stb_image.h>
#include <iostream>


Skybox::Skybox()
        : VAO(0), VBO(0), textureIDs(), shaderID(0),skyboxIndex(-1) {
    skyboxVertices = {
            // positions for the cube
            -1.0f,  1.0f, -1.0f, // top-left
            -1.0f, -1.0f, -1.0f, // bottom-left
            1.0f, -1.0f, -1.0f, // bottom-right
            1.0f, -1.0f, -1.0f, // bottom-right
            1.0f,  1.0f, -1.0f, // top-right
            -1.0f,  1.0f, -1.0f, // top-left

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };
}

void Skybox::initialise(GLuint shaderID) {
    this->shaderID = shaderID;
    //textureID = loadCubemap(paths);
    setupSkybox();
}

void Skybox::setupSkybox() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(float), &skyboxVertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindVertexArray(0);  // Unbind the VAO to avoid accidental modifications
}


GLuint Skybox::addSkybox(const std::vector<std::string>& paths){
    GLuint tID = loadCubemap(paths);
    textureIDs.push_back(tID);
    if (skyboxIndex == -1){skyboxIndex = 0;}
    return textureIDs.size() - 1;
}

GLuint Skybox::loadCubemap(const std::vector<std::string>& paths) {
    if (paths.size() != 6) {
        std::cout << "Error: A cubemap requires exactly 6 images!" << std::endl;
        return 0;
    }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, channels;

    for (GLuint i = 0; i < 6; i++) {
        float* data = stbi_loadf(paths[i].c_str(), &width, &height, &channels, 0);

        if (data) {
            int format;
            if (channels == 1) {
                format = GL_RED;
            } else if (channels == 2) {
                format = GL_RG;
            } else if (channels == 3) {
                format = GL_RGB;
            } else if (channels == 4) {
                format = GL_RGBA;
            } else {
                std::cout << "Unsupported number of channels: " << channels << std::endl;
            }

            // Assign the image data to the correct face of the cubemap
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, width, height, 0, format, GL_FLOAT, data);
            stbi_image_free(data);
        } else {
            std::cout << "Failed to load texture: " << paths[i] << std::endl;
            return 0; // Return 0 if any texture fails to load
        }
    }

    // Set the texture parameters for the cubemap
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Generate mipmaps for the cubemap texture
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


    return textureID;
}

void Skybox::render(const glm::mat4& view, const glm::mat4& projection) {
    if (skyboxIndex != -1 && skyboxIndex < textureIDs.size()) {
        //glDepthFunc(GL_LEQUAL);  // Change depth function so skybox is drawn at the farthest depth

        // Remove translation from the view matrix
        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));

        // Use the skybox shader
        glUseProgram(shaderID);
        glDisable(GL_DEPTH_TEST);  // Re-enable depth writing
        // Set the view and projection matrices in the shader
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, glm::value_ptr(viewNoTranslation));
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
        // Bind the skybox VAO


        // Bind the cubemap texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureIDs[skyboxIndex]);
        glUniform1i(glGetUniformLocation(shaderID, "skybox"), 0);

        // Draw the skybox

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glEnable(GL_DEPTH_TEST);  // Re-enable depth writing
        //glBindVertexArray(0);
        glDisableVertexAttribArray(0);
        //glBindVertexArray(0);
        // Reset depth function
        //glDepthFunc(GL_LESS);  // Set depth function back to default
    }
}
