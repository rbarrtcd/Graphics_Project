//
// Created by rowan on 22/12/2024.
//

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include <stb/stb_image.h>
#include "stb_image_write.h"

glm::mat4 computeModelMatrix(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
    // First create the individual transformation matrices
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position); // Create translation matrix
    glm::mat4 rotationMatrixX = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around x-axis
    glm::mat4 rotationMatrixY = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around y-axis
    glm::mat4 rotationMatrixZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around z-axis
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale); // Create scale matrix

    // Multiply the matrices together in the correct order: scale -> rotate -> translate
    return translationMatrix * rotationMatrixZ * rotationMatrixY * rotationMatrixX * scaleMatrix;
}

void computeNormals(const GLfloat* vertexBuffer, const GLuint* indexBuffer,
                    int vertexCount, int indexCount, GLfloat* normalBuffer) {
    // Initialize normalBuffer to zero
    memset(normalBuffer, 0, sizeof(GLfloat) * vertexCount * 3);

    // Iterate through each triangle (3 indices form a triangle)
    for (int i = 0; i < indexCount; i += 3) {
        // Get the indices of the vertices forming this triangle
        GLint idx0 = indexBuffer[i];
        GLint idx1 = indexBuffer[i + 1];
        GLint idx2 = indexBuffer[i + 2];

        // Retrieve the positions of the vertices
        glm::vec3 v0(vertexBuffer[idx0 * 3], vertexBuffer[idx0 * 3 + 1], vertexBuffer[idx0 * 3 + 2]);
        glm::vec3 v1(vertexBuffer[idx1 * 3], vertexBuffer[idx1 * 3 + 1], vertexBuffer[idx1 * 3 + 2]);
        glm::vec3 v2(vertexBuffer[idx2 * 3], vertexBuffer[idx2 * 3 + 1], vertexBuffer[idx2 * 3 + 2]);

        // Compute the edges of the triangle
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;

        // Compute the face normal (cross product of the edges)
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        // Add the face normal to each vertex normal of the triangle
        normalBuffer[idx0 * 3]     += faceNormal.x;
        normalBuffer[idx0 * 3 + 1] += faceNormal.y;
        normalBuffer[idx0 * 3 + 2] += faceNormal.z;

        normalBuffer[idx1 * 3]     += faceNormal.x;
        normalBuffer[idx1 * 3 + 1] += faceNormal.y;
        normalBuffer[idx1 * 3 + 2] += faceNormal.z;

        normalBuffer[idx2 * 3]     += faceNormal.x;
        normalBuffer[idx2 * 3 + 1] += faceNormal.y;
        normalBuffer[idx2 * 3 + 2] += faceNormal.z;
    }

    // Normalize the accumulated normals for each vertex
    for (int i = 0; i < vertexCount; ++i) {
        glm::vec3 normal(normalBuffer[i * 3], normalBuffer[i * 3 + 1], normalBuffer[i * 3 + 2]);
        normal = glm::normalize(normal);

        normalBuffer[i * 3]     = normal.x;
        normalBuffer[i * 3 + 1] = normal.y;
        normalBuffer[i * 3 + 2] = normal.z;
    }
}

void transformNormals(GLfloat* normalBuffer, int normalCount, const glm::mat4& transformMatrix) {
    glm::mat4 rotationScaleMatrix = glm::mat4(transformMatrix);
    rotationScaleMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Remove translation part (set translation to zero)

    for (int i = 0; i < normalCount; ++i) {
        glm::vec3 normal(normalBuffer[i * 3], normalBuffer[i * 3 + 1], normalBuffer[i * 3 + 2]);

        normal = glm::normalize(glm::mat3(rotationScaleMatrix) * normal);

        normalBuffer[i * 3]     = normal.x;
        normalBuffer[i * 3 + 1] = normal.y;
        normalBuffer[i * 3 + 2] = normal.z;
    }
}


GLuint LoadTextureTileBox(const char *texture_file_path) {
    int w, h, channels;
    uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // To tile textures on a box, we set wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (img) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture " << texture_file_path << std::endl;
    }
    stbi_image_free(img);

    return texture;
}
