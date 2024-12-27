//
// Created by rowan on 22/12/2024.
//

#include "primitives.h"

#ifndef LAB4_GEOMETRY_H
#define LAB4_GEOMETRY_H

class Geometry {
public:
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
    GLuint numVertices;
    GLuint numIndices;
    GLfloat * vertex_buffer_data;
    GLfloat * normal_buffer_data;
    GLfloat * color_buffer_data;
    GLuint * index_buffer_data;
    GLfloat * uv_buffer_data;

    // OpenGL buffers
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint colorBufferID;
    GLuint normalBufferID;
    GLuint uvBufferID;
    GLuint textureID;
    GLuint programID;

    // Shader variable IDs
    GLuint mvpMatrixID;
    GLuint modelMatrixID;
    GLuint textureSamplerID;

    glm::mat4 modelMatrix;

    Geometry(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, GLuint programID);

    void initialize(MeshData meshData, GLuint texture_ID);
    void render(glm::mat4 cameraMatrix);
    void lightRender(GLuint lightShader, Light light);
    void cleanup();


};

#endif //LAB4_GEOMETRY_H
