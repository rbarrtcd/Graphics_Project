//
// Created by rowan on 09/12/2024.
//

#ifndef LAB4_ANIMATIONDATA_H
#define LAB4_ANIMATIONDATA_H


// Shadow mapping

#include <vector>
#include <unordered_map>
#include <string>
#include "glm/gtc/quaternion.hpp"


struct BoneInfo {
    int index;
    glm::mat4 localTransform;
    glm::mat4 offsetMatrix;  // Inverse bind matrix
    glm::mat4 finalTransformation; // Final transformation for this bone
};

// Keyframe data
struct Keyframe {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

// Animation data for a bone
struct BoneAnimation {
    std::string name;
    std::vector<float> keyframeTimes;
    std::vector<Keyframe> keyframes;
};

// Vertex data
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 boneWeights; // Up to 4 bone weights
    glm::ivec4 boneIDs;    // Indices of the bones affecting this vertex
};

struct AnimationTrack {
    std::vector<BoneAnimation> boneAnims;
};


// Combined animation data
struct FileAnimationData {
    std::unordered_map<std::string, BoneInfo> boneMap; // Bone name -> BoneInfo
    std::vector<AnimationTrack>animations;          // Animation tracks
    std::vector<Vertex> vertices;                    // Mesh vertices
    std::vector<unsigned int> indices;               // Mesh indices
};

struct BoneNode {
    int index;                        // Index of this bone in the bone list
    std::string name;                 // Bone name
    glm::mat4 offsetMatrix;           // Inverse bind matrix
    glm::mat4 localTransform;         // Local transform from animations
    std::vector<int> childrenIndices; // Indices of child bones
    glm::mat4 finalTransformation; // Final transformation for this bone
};


struct AnimationData {
    AnimationData()
            : rootIndex(-1) // Initialize rootIndex to a default value (you can set it to whatever makes sense)
    {
        // Vectors are automatically initialized to empty
        // No need to manually initialize them, but can set initial sizes if needed
    }

    std::vector<BoneNode> bones;      // All bones in a hierarchy
    std::vector<Vertex> vertices;     // Mesh vertices
    std::vector<unsigned int> indices; // Mesh indices
    std::vector<AnimationTrack>animations;
    int rootIndex;
};




AnimationData deepCopyAnimData(AnimationData& animData);


#endif //LAB4_ANIMATIONDATA_H
