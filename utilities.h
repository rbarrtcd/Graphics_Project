//
// Created by rowan on 22/12/2024.
//

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "animationData.h"
#ifndef LAB4_UTILITIES_H
#define LAB4_UTILITIES_H



glm::mat4 computeModelMatrix(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);

void computeNormals(const GLfloat* vertexBuffer, const GLuint* indexBuffer, int vertexCount, int indexCount, GLfloat* normalBuffer);

void transformNormals(GLfloat* normalBuffer, int normalCount, const glm::mat4& transformMatrix);

GLuint LoadTextureTileBox(const char *texture_file_path);

void saveGBufferTextures(GLuint gBuffer, GLuint gColour, GLuint gPosition, GLuint gNormal, GLuint rboDepth, GLuint emit, int width, int height);

void saveDepthCubemapToImage(GLuint depthCubemap, const std::string& fileName);

void saveColorCubemapToImage(GLuint colorCubemap, const std::string& fileName);

void printMatrix(const glm::mat4& matrix);

void printMatrixVector(const std::vector<glm::mat4>& matrices);

bool loadFbx(const std::string& fbxFilePath,
             std::vector<GLfloat>& vertices,
             std::vector<GLfloat>& normals,
             std::vector<GLfloat>& uvs,
             std::vector<GLuint>& indices,
             std::vector<GLfloat>& colors);

AnimationData buildBoneHierarchy(const aiScene* scene, FileAnimationData& fileAnimationData);

AnimationData loadFBXAnimation(const std::string& filePath);

void createGBuffer(GLuint* gBuffer, GLuint* gColour, GLuint* gPosition, GLuint* gNormal, GLuint* rboDepth, GLuint* emit, int width, int height);
#endif //LAB4_UTILITIES_H

