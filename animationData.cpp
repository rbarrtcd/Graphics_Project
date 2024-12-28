//
// Created by rowan on 28/12/2024.
//
#include "animationData.h"
#include <vector>
#include <unordered_map>
#include "glm/gtc/quaternion.hpp"

AnimationData deepCopyAnimData(AnimationData& animData) {
    // Create a new AnimationData object to store the copy (by value, no need for new)
    AnimationData newAnimData;

    // Copy bones
    newAnimData.bones = animData.bones;  // Vector copy handles deep copying elements inside

    // Copy vertices
    newAnimData.vertices = animData.vertices;

    // Copy indices
    newAnimData.indices = animData.indices;

    // Copy animations with deep copies of each BoneAnimation and Keyframe
    newAnimData.animations.resize(animData.animations.size());
    for (size_t i = 0; i < animData.animations.size(); ++i) {
        const AnimationTrack& track = animData.animations[i];
        AnimationTrack& newTrack = newAnimData.animations[i];

        newTrack.boneAnims.resize(track.boneAnims.size());
        for (size_t j = 0; j < track.boneAnims.size(); ++j) {
            const BoneAnimation& boneAnim = track.boneAnims[j];
            BoneAnimation& newBoneAnim = newTrack.boneAnims[j];

            newBoneAnim.name = boneAnim.name;
            newBoneAnim.keyframeTimes = boneAnim.keyframeTimes;
            newBoneAnim.keyframes.resize(boneAnim.keyframes.size());

            for (size_t k = 0; k < boneAnim.keyframes.size(); ++k) {
                newBoneAnim.keyframes[k] = boneAnim.keyframes[k];  // Keyframe copy
            }
        }
    }

    // Copy rootIndex
    newAnimData.rootIndex = animData.rootIndex;

    // Return the copied object by value
    return newAnimData;
}


