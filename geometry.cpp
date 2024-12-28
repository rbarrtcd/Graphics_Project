//
// Created by rowan on 22/12/2024.
//

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "animationData.h"
#include "glm/gtc/type_ptr.hpp"
#include <stb/stb_image.h>
#include "stb_image_write.h"
#include <iostream>

#include "utilities.h"
#include "light.h"
#include "primitives.h"
#include "geometry.h"


Geometry::Geometry(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, GLuint programID) {
    this->position = position;
    this->scale = scale;
    this->rotation = rotation;
    this->programID = programID;
}

void Geometry::initialize(MeshData meshData, GLuint texture_ID) {
    this->numVertices = meshData.numVertices;
    this->numIndices = meshData.numIndices;

    this->vertex_buffer_data = new GLfloat[meshData.numVertices * 3];  // 3 floats per vertex (x, y, z)
    this->normal_buffer_data = new GLfloat[meshData.numVertices * 3];   // 3 floats per normal (x, y, z)
    this->color_buffer_data = new GLfloat[meshData.numVertices * 3];    // 3 floats per color (r, g, b)
    this->index_buffer_data = new GLuint[meshData.numIndices];          // One GLuint per index
    this->uv_buffer_data = new GLfloat[meshData.numVertices * 2];       // 2 floats per UV (u, v)

    // Copy the data from meshData into the new buffers
    std::memcpy(this->vertex_buffer_data, meshData.vertex_buffer_data, sizeof(GLfloat) * meshData.numVertices * 3);
    std::memcpy(this->normal_buffer_data, meshData.normal_buffer_data, sizeof(GLfloat) * meshData.numVertices * 3);
    std::memcpy(this->color_buffer_data, meshData.color_buffer_data, sizeof(GLfloat) * meshData.numVertices * 3);
    std::memcpy(this->index_buffer_data, meshData.index_buffer_data, sizeof(GLuint) * meshData.numIndices);
    std::memcpy(this->uv_buffer_data, meshData.uv_buffer_data, sizeof(GLfloat) * meshData.numVertices * 2);


    computeNormals(vertex_buffer_data, index_buffer_data, numVertices, numIndices, normal_buffer_data);
    modelMatrix = computeModelMatrix(position, rotation, scale);
    transformNormals(normal_buffer_data, numVertices, modelMatrix);

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // Vertex Buffer
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW);

    // Color Buffer
    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
    glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);

    // UV Buffer
    glGenBuffers(1, &uvBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glBufferData(GL_ARRAY_BUFFER, numVertices * 2 * sizeof(GLfloat), uv_buffer_data, GL_STATIC_DRAW);

    // Normal Buffer
    glGenBuffers(1, &normalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(GLfloat), normal_buffer_data, GL_STATIC_DRAW);

    // Index Buffer
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), index_buffer_data, GL_STATIC_DRAW);

    // Shader handles
    mvpMatrixID = glGetUniformLocation(programID, "MVP");
    modelMatrixID = glGetUniformLocation(programID, "modelMatrix");

    //textureID = LoadTextureTileBox(texture_file_path);
    textureID = texture_ID;
    textureSamplerID = glGetUniformLocation(programID, "textureSampler");
}


void Geometry::render(glm::mat4 cameraMatrix) {
    glUseProgram(programID);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Enable UV buffer and texture sampler
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

    // Set model-view-projection matrix
    glm::mat4 mvp = cameraMatrix * modelMatrix;
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

    // Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(textureSamplerID, 0);

    // Draw the geometry
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, (void*)0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}

void Geometry::lightRender(GLuint lightShader, Light light) {
    glUseProgram(lightShader);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glm::mat4 modelMatrix = computeModelMatrix(position, rotation, scale);
    glm::mat4 MVP =  light.VPmatrix * modelMatrix;
    if (light.lightType == POINT_LIGHT) {
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "modelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "shadowMatrices"), 6, GL_FALSE,
                           &light.VPmatrices[0][0][0]);
    } else{
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "modelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "MVP"), 1, GL_FALSE, &MVP[0][0]);
    }
    glUniform3fv(glGetUniformLocation(lightShader, "lightPos"), 1, &light.position[0]);
    glUniform1f(glGetUniformLocation(lightShader, "farPlane"), light.lightRange);

    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, (void*)0);

    glDisableVertexAttribArray(0);
}

void Geometry::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &colorBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteBuffers(1, &uvBufferID);
    glDeleteTextures(1, &textureID);
}



