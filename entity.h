//
// Created by rowan on 22/12/2024.
//
#ifndef LAB4_ENTITY_H
#define LAB4_ENTITY_H
#include "AnimationData.h"
#include "light.h"

class Entity {
public:
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;

    GLuint numVertices;
    GLuint numIndices;

    GLfloat *vertex_buffer_data;
    GLfloat *normal_buffer_data;
    GLfloat *color_buffer_data;
    GLuint *index_buffer_data;
    GLfloat *uv_buffer_data;

    // OpenGL buffers
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint colorBufferID;
    GLuint normalBufferID;
    GLuint uvBufferID;
    GLuint textureID;

    GLuint staticID;
    GLuint animatedID;

    // Shader variable IDs
    GLuint mvpMatrixID;
    GLuint textureSamplerID;
    GLuint modelMatrixID;
    GLuint AnimatedmvpMatrixID;
    GLuint AnimatedmodelMatrixID;
    GLuint AnimatedvertexArrayID;
    GLuint jointMatricesID;
    GLuint boneWeightsID;
    GLuint boneIndicesID;

    glm::mat4 modelMatrix;

    std::vector<glm::vec4> boneWeightsData;
    std::vector<glm::vec4> boneIndicesData;
    std::vector<Vertex> copiedVertices;

    AnimationData animationData;
    int currentAnimationTrack = -1;
    float animationTime = 0.0f;
    bool isAnimationPlaying = false;

    Entity(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, GLuint staticShader, GLuint animatedShader);

    void loadModelData(const std::vector<GLfloat> &vertices,
                       const std::vector<GLfloat> &normals,
                       const std::vector<GLfloat> &colors,
                       const std::vector<GLuint> &indices,
                       const std::vector<GLfloat> &uvs);

    void initializeBuffers();

    void setTexture(const std::string &texturePath);
    void Entity::setTexture(GLuint textureID);

    void loadAnimationData(AnimationData animationData);

    void playAnimation(int track);

    void computeGlobalBoneTransform(
            const AnimationData &animationData,
            const std::vector<glm::mat4> &localTransforms,
            int boneIndex,
            const glm::mat4 &parentTransform,
            std::vector<glm::mat4> &globalTransforms
    );

    void update();

    int findKeyframeIndex(const std::vector<float> &times, float animationTime);

    void updateAnimation(float time, std::vector<glm::mat4> &nodeTransforms);

    void render(glm::mat4 cameraMatrix);

    void lightRender(GLuint lightShader, GLuint entityLighting, Light light);

    void cleanup();
};



#endif //LAB4_ENTITY_H
