//
// Created by rowan on 22/12/2024.
//
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtc/type_ptr.hpp"

#include <string>
#include <iostream>

#include <stb/stb_image.h>
#include "stb_image_write.h"

#include "light.h"
#include "entity.h"
#include "utilities.h"
#include "animationData.h"



static GLuint LoadTextureTileBox(const char *texture_file_path) {
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




Entity::Entity(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, GLuint staticShader, GLuint animatedShader) {
    this->position = position;
    this->scale = scale;
    this->rotation = rotation;
    this->staticID = staticShader;
    this->animatedID = animatedShader;
}

void Entity::loadModelData(const std::vector<GLfloat>& vertices,
                   const std::vector<GLfloat>& normals,
                   const std::vector<GLfloat>& colors,
                   const std::vector<GLuint>& indices,
                   const std::vector<GLfloat>& uvs) {
    // Set the number of vertices and indices
    numVertices = vertices.size() / 3; // Assuming 3 components per vertex (x, y, z)
    numIndices = indices.size();

    // Allocate memory and copy vertex data
    vertex_buffer_data = new GLfloat[vertices.size()];
    std::copy(vertices.begin(), vertices.end(), vertex_buffer_data);

    // Allocate memory and copy normal data
    normal_buffer_data = new GLfloat[normals.size()];
    std::copy(normals.begin(), normals.end(), normal_buffer_data);

    // Allocate memory and copy color data
    color_buffer_data = new GLfloat[colors.size()];
    std::copy(colors.begin(), colors.end(), color_buffer_data);

    // Allocate memory and copy index data
    index_buffer_data = new GLuint[indices.size()];
    std::copy(indices.begin(), indices.end(), index_buffer_data);

    // Allocate memory and copy UV data
    uv_buffer_data = new GLfloat[uvs.size()];
    std::copy(uvs.begin(), uvs.end(), uv_buffer_data);


    //transformNormals(normal_buffer_data, numVertices, computeModelMatrix(position, rotation, scale));
    initializeBuffers();
    mvpMatrixID = glGetUniformLocation(staticID, "MVP");
    modelMatrixID = glGetUniformLocation(staticID, "modelMatrix");

    AnimatedmvpMatrixID = glGetUniformLocation(animatedID, "MVP");
    AnimatedmodelMatrixID = glGetUniformLocation(animatedID, "modelMatrix");
    jointMatricesID = glGetUniformLocation(animatedID, "jointMatrix");

}

void Entity::initializeBuffers(){
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // Create a vertex buffer object to store the vertex data
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, numVertices*3*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW);

    // Create a vertex buffer object to store the color data
    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
    glBufferData(GL_ARRAY_BUFFER, numVertices*3*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);

    // Create a vertex buffer object to store the UV data
    glGenBuffers(1, &uvBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glBufferData(GL_ARRAY_BUFFER, numVertices*2*sizeof(GLfloat), uv_buffer_data, GL_STATIC_DRAW);

    // Create an index buffer object to store the index data that defines triangle faces
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices*sizeof(GLfloat), index_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &normalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glBufferData(GL_ARRAY_BUFFER, numVertices*3*sizeof(GLfloat), normal_buffer_data, GL_STATIC_DRAW);

    //animation
    glGenVertexArrays(1, &AnimatedvertexArrayID);
    glBindVertexArray(AnimatedvertexArrayID);



}

void Entity::setTexture(const std::string& texturePath){
    textureID = LoadTextureTileBox(texturePath.c_str());
}

void Entity::setTexture(GLuint newTextureID){
    textureID = newTextureID;
}

void Entity::loadAnimationData(AnimationData animationData) {
    this->animationData = animationData;

    for (auto& vertex : animationData.vertices) {
        glm::vec4 floatIndices = glm::vec4((GLfloat) vertex.boneIDs.x, (GLfloat) vertex.boneIDs.y, (GLfloat) vertex.boneIDs.z, (GLfloat) vertex.boneIDs.w);
        if (vertex.boneWeights.x < 0.0) {vertex.boneWeights.x = 0; floatIndices.x = 0.0;}
        if (vertex.boneWeights.y < 0.0) {vertex.boneWeights.y = 0; floatIndices.y = 0.0;}
        if (vertex.boneWeights.z < 0.0) {vertex.boneWeights.z = 0; floatIndices.z = 0.0;}
        if (vertex.boneWeights.w < 0.0) {vertex.boneWeights.w = 0; floatIndices.w = 0.0;}
        boneWeightsData.push_back(vertex.boneWeights);
        boneIndicesData.push_back(floatIndices);


    }

    glGenBuffers(1, &boneWeightsID);
    glBindBuffer(GL_ARRAY_BUFFER, boneWeightsID);
    glBufferData(GL_ARRAY_BUFFER, boneWeightsData.size() * sizeof(glm::vec4), boneWeightsData.data(), GL_STATIC_DRAW);

    // Create a vertex buffer object to store the UV data
    glGenBuffers(1, &boneIndicesID);
    glBindBuffer(GL_ARRAY_BUFFER, boneIndicesID);
    glBufferData(GL_ARRAY_BUFFER, boneIndicesData.size() * sizeof(glm::vec4), boneIndicesData.data(), GL_STATIC_DRAW);


}

void Entity::playAnimation(int track){
    currentAnimationTrack = track;
    animationTime = glfwGetTime();
    isAnimationPlaying = true;
}





void Entity::computeGlobalBoneTransform(
        const AnimationData& animationData,
        const std::vector<glm::mat4>& localTransforms,
        int boneIndex,
        const glm::mat4& parentTransform,
        std::vector<glm::mat4>& globalTransforms
) {
    if (boneIndex == -1){return;}
    // Get the current bone
    const BoneNode& bone = animationData.bones[boneIndex];

    // Calculate the global transformation for this bone
    glm::mat4 globalTransform = parentTransform * localTransforms[boneIndex];

    // Store the global transformation
    globalTransforms[boneIndex] = globalTransform;

    // Recursively compute transformations for child bones
    for (int childIndex : bone.childrenIndices) {
        computeGlobalBoneTransform(animationData, localTransforms, childIndex, globalTransform, globalTransforms);
    }
}



void Entity::update() {
    if (currentAnimationTrack >= 0 && isAnimationPlaying) {
        // Advance animation time
        animationTime += (glfwGetTime()-animationTime);

        std::vector<glm::mat4> nodeTransforms(animationData.bones.size());
        for (size_t i = 0; i < nodeTransforms.size(); ++i) {
            nodeTransforms[i] = glm::mat4(1.0);
        }
        updateAnimation(animationTime, nodeTransforms);
        std::vector<glm::mat4> globalTransforms(animationData.bones.size(), glm::mat4(1.0f));
        computeGlobalBoneTransform(animationData, nodeTransforms, animationData.rootIndex, glm::mat4(1.0f), globalTransforms);

        for (size_t i = 0; i < animationData.bones.size(); ++i) {
            const BoneNode& bone = animationData.bones[i];
            const glm::mat4& globalTransform = globalTransforms[i];
            const glm::mat4& offsetMatrix = bone.offsetMatrix;

            animationData.bones[i].finalTransformation = globalTransform * offsetMatrix;

        }
    }
}

int Entity::findKeyframeIndex(const std::vector<float>& times, float animationTime)
{
    int left = 0;
    int right = times.size() - 1;

    while (left <= right) {
        int mid = (left + right) / 2;

        if (mid + 1 < times.size() && times[mid] <= animationTime && animationTime < times[mid + 1]) {
            return mid;
        }
        else if (times[mid] > animationTime) {
            right = mid - 1;
        }
        else { // animationTime >= times[mid + 1]
            left = mid + 1;
        }
    }

    // Target not found
    return times.size() - 2;
}

void Entity::updateAnimation(float time, std::vector<glm::mat4> &nodeTransforms) {
    if (currentAnimationTrack < 0 || currentAnimationTrack >= animationData.animations.size()) {
        return; // Invalid track, do nothing
    }
    const AnimationTrack &animation = animationData.animations[currentAnimationTrack];
    // Precompute animation time (looping back when it exceeds duration)
    const std::vector<float> &keyframeTimes = animation.boneAnims[0].keyframeTimes;
    if (keyframeTimes.empty()) {
        return; // No keyframes, do nothing
    }

    float animationTime = fmod(time, keyframeTimes.back());

    for (const auto &boneNode : animationData.bones) {
        glm::mat4 localTransform = glm::mat4(1.0f);

        glm::mat4 localTransformPos = glm::mat4(1.0f);
        glm::mat4 localTransformRot = glm::mat4(1.0f);
        glm::mat4 localTransformSca = glm::mat4(1.0f);

        // Find corresponding animation track for this bone
        auto it = std::find_if(animationData.animations[currentAnimationTrack].boneAnims.begin(),
                               animationData.animations[currentAnimationTrack].boneAnims.end(),
                               [&boneNode](const BoneAnimation &anim) { return anim.name == boneNode.name; });

        if (it != animationData.animations[currentAnimationTrack].boneAnims.end()) {
            const BoneAnimation &boneAnim = *it;
            int keyframeIndex = findKeyframeIndex(boneAnim.keyframeTimes, animationTime);

            float t = 0.0f;
            if (keyframeIndex + 1 < boneAnim.keyframeTimes.size()) {
                float t0 = boneAnim.keyframeTimes[keyframeIndex];
                float t1 = boneAnim.keyframeTimes[keyframeIndex + 1];
                t = (animationTime - t0) / (t1 - t0);
            }
            //keyframeIndex = 0;
            // Interpolate keyframes for this bone
            const Keyframe &keyframe0 = boneAnim.keyframes[keyframeIndex];
            const Keyframe &keyframe1 = boneAnim.keyframes[keyframeIndex + 1];



            glm::vec3 interpolatedPosition = glm::mix(keyframe0.position, keyframe1.position, t);
            glm::quat interpolatedRotation = glm::slerp(keyframe0.rotation, keyframe1.rotation, t);
            glm::vec3 interpolatedScale = glm::mix(keyframe0.scale, keyframe1.scale, t);

            localTransformPos = glm::translate(glm::mat4(1.0f), interpolatedPosition);
            localTransformRot = glm::mat4_cast(interpolatedRotation);
            localTransformSca = glm::scale(glm::mat4(1.0f), interpolatedScale);

            localTransform = localTransformPos * localTransformRot * localTransformSca;
            nodeTransforms[boneNode.index] = localTransform;
        }



    }
}



void Entity::render(glm::mat4 cameraMatrix) {
    if (isAnimationPlaying){
        glUseProgram(animatedID);


        modelMatrix = computeModelMatrix(position, rotation, scale);


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


        // Pass bone weights to the shader
        glEnableVertexAttribArray(4); // Enable the attribute for bone weights
        glBindBuffer(GL_ARRAY_BUFFER, boneWeightsID); // Assuming the buffer is already created and bound
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); // Location 3 for bone weights


        // Pass bone indices to the shader
        glEnableVertexAttribArray(5); // Enable the attribute for bone indices
        glBindBuffer(GL_ARRAY_BUFFER, boneIndicesID); // Assuming the buffer is already created and bound
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 0, 0); // Location 4 for bone indices


        std::vector<float> jointMatrixData;
        for (const auto& bone : animationData.bones) {
            const float* matPtr = glm::value_ptr(bone.finalTransformation);
            jointMatrixData.insert(jointMatrixData.end(), matPtr, matPtr + 16);
        }

        glUniformMatrix4fv(jointMatricesID, animationData.bones.size(), GL_FALSE, jointMatrixData.data());


        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);


        // Set model-view-projection matrix
        glm::mat4 mvp = cameraMatrix * modelMatrix;
        glUniformMatrix4fv(AnimatedmvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
        glUniformMatrix4fv(AnimatedmodelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

        // Set textureSampler to use texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(textureSamplerID, 0);

        // Draw the box
        glDrawElements(
                GL_TRIANGLES,      // mode
                numIndices,        // number of indices
                GL_UNSIGNED_INT,   // type
                (void *) 0           // element array buffer offset
        );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(3);
        glDisableVertexAttribArray(4);
        glDisableVertexAttribArray(5);
    } else {
        glUseProgram(staticID);


        modelMatrix = computeModelMatrix(position, rotation, scale);


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

        // Set textureSampler to use texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(textureSamplerID, 0);

        // Draw the box
        glDrawElements(
                GL_TRIANGLES,      // mode
                numIndices,        // number of indices
                GL_UNSIGNED_INT,   // type
                (void *) 0           // element array buffer offset
        );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(3);
    }
}

void Entity::lightRender(GLuint lightShader, GLuint entityLighting, Light light) {
    if (isAnimationPlaying) {
        glUseProgram(entityLighting);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Pass bone weights to the shader
        glEnableVertexAttribArray(1); // Enable the attribute for bone weights
        glBindBuffer(GL_ARRAY_BUFFER, boneWeightsID); // Assuming the buffer is already created and bound
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); // Location 3 for bone weights


        // Pass bone indices to the shader
        glEnableVertexAttribArray(2); // Enable the attribute for bone indices
        glBindBuffer(GL_ARRAY_BUFFER, boneIndicesID); // Assuming the buffer is already created and bound
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0); // Location 4 for bone indices


        std::vector<float> jointMatrixData;
        for (const auto& bone : animationData.bones) {
            const float* matPtr = glm::value_ptr(bone.finalTransformation);
            jointMatrixData.insert(jointMatrixData.end(), matPtr, matPtr + 16);
        }

        glUniformMatrix4fv(glGetUniformLocation(entityLighting, "jointMatrix"), animationData.bones.size(), GL_FALSE, jointMatrixData.data());


        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

        glm::mat4 modelMatrix = computeModelMatrix(position, rotation, scale);
        glm::mat4 MVP =  light.VPmatrix * modelMatrix;
        glUniformMatrix4fv(glGetUniformLocation(entityLighting, "modelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
        if (light.lightType == POINT_LIGHT) {
            glUniformMatrix4fv(glGetUniformLocation(entityLighting, "shadowMatrices"), 6, GL_FALSE,
                               &light.VPmatrices[0][0][0]);
        } else{
            glUniformMatrix4fv(glGetUniformLocation(entityLighting, "MVP"), 1, GL_FALSE, &MVP[0][0]);
        }

        glUniform3fv(glGetUniformLocation(entityLighting, "lightPos"), 1, &light.position[0]);
        glUniform1f(glGetUniformLocation(entityLighting, "farPlane"), light.lightRange);

        // Draw the box
        glDrawElements(
                GL_TRIANGLES,      // mode
                numIndices,        // number of indices
                GL_UNSIGNED_INT,   // type
                (void *) 0           // element array buffer offset
        );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    } else {
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

        // Draw the box
        glDrawElements(
                GL_TRIANGLES,      // mode
                numIndices,        // number of indices
                GL_UNSIGNED_INT,   // type
                (void *) 0           // element array buffer offset
        );

        glDisableVertexAttribArray(0);
    }
}



void Entity::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &colorBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteBuffers(1, &uvBufferID);
    glDeleteTextures(1, &textureID);
}

