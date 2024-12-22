//
// Created by rowan on 22/12/2024.
//

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#ifndef LAB4_UTILITIES_H
#define LAB4_UTILITIES_H

glm::mat4 computeModelMatrix(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);

void computeNormals(const GLfloat* vertexBuffer, const GLuint* indexBuffer, int vertexCount, int indexCount, GLfloat* normalBuffer);

void transformNormals(GLfloat* normalBuffer, int normalCount, const glm::mat4& transformMatrix);

GLuint LoadTextureTileBox(const char *texture_file_path);

#endif //LAB4_UTILITIES_H
