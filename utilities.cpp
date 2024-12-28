//
// Created by rowan on 22/12/2024.
//

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include <stb/stb_image.h>
#include <vector>
#include <iomanip>
#include "stb_image_write.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <functional>

#include "animationData.h"
#include "glm/gtc/type_ptr.hpp"

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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


void saveGBufferTextures(GLuint gBuffer, GLuint gColour, GLuint gPosition, GLuint gNormal, GLuint rboDepth, GLuint emit, int width, int height) {
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // Allocate space for texture data
    unsigned char* colourData = new unsigned char[width * height * 4]; // RGBA
    float* positionData = new float[width * height * 3]; // XYZ as floats
    float* normalData = new float[width * height * 3]; // XYZ as floats
    float * depthData = new float[width * height]; // Depth as grayscale (0-255)
    float* emitData = new float[width * height];

    // Get colour texture data
    glBindTexture(GL_TEXTURE_2D, gColour);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, colourData);


    // Get position texture data
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, positionData);

    // Get normal texture data
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, normalData);

    // Get depth texture data (renderbuffer)
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depthData);

    glBindTexture(GL_TEXTURE_2D, emit);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, emitData);
    // Save colour as PNG
    stbi_write_png("gBuffer_colour.png", width, height, 4, colourData, width * 4);

    // Save position as PNG (use normalized values for RGBA)
    unsigned char* positionNormalized = new unsigned char[width * height * 3];
    for (int i = 0; i < width * height; ++i) {
        positionNormalized[i * 3] = (unsigned char)(std::min(1.0f, positionData[i * 3]) * 255);     // Red (X)
        positionNormalized[i * 3 + 1] = (unsigned char)(std::min(1.0f, positionData[i * 3 + 1]) * 255); // Green (Y)
        positionNormalized[i * 3 + 2] = (unsigned char)(std::min(1.0f, positionData[i * 3 + 2]) * 255); // Blue (Z)
    }
    stbi_write_png("gBuffer_position.png", width, height, 3, positionNormalized, width * 3);

    // Save normal as PNG (use normalized values for RGBA)
    unsigned char* normalNormalized = new unsigned char[width * height * 3];
    for (int i = 0; i < width * height; ++i) {
        normalNormalized[i * 3] = (unsigned char)((normalData[i * 3] + 1.0f) * 0.5f * 255);     // Red (X)
        normalNormalized[i * 3 + 1] = (unsigned char)((normalData[i * 3 + 1] + 1.0f) * 0.5f * 255); // Green (Y)
        normalNormalized[i * 3 + 2] = (unsigned char)((normalData[i * 3 + 2] + 1.0f) * 0.5f * 255); // Blue (Z)
    }
    stbi_write_png("gBuffer_normal.png", width, height, 3, normalNormalized, width * 3);
    //std::cout << "depth[0] = " << (float) depthData[0] << std::endl;
    // Save depth as PNG (normalize to 0-255 range)
    unsigned char* depthNormalized = new unsigned char[width * height];
    for (int i = 0; i < width * height; ++i) {
        depthNormalized[i] = (unsigned char)(depthData[i] * 255); // Depth range (0-1) to (0-255)
    }
    stbi_write_png("gBuffer_depth.png", width, height, 1, depthNormalized, width);

    unsigned char* emitImage = new unsigned char[width * height]; // RGB for emit
    for (int i = 0; i < width * height; ++i) {
        unsigned char value = (emitData[i] > 0.5f) ? 255 : 0; // 0.5 threshold to decide black or white
        emitImage[i] = value;     // Red (same for all channels)
    }
    stbi_write_png("gBuffer_emit.png", width, height, 1, emitImage, width);


    // Clean up
    delete[] colourData;
    delete[] positionData;
    delete[] normalData;
    delete[] depthData;
    delete[] emitData;
    delete[] positionNormalized;
    delete[] normalNormalized;
    delete[] depthNormalized;
    delete[] emitImage;

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind framebuffer
}

void saveDepthCubemapToImage(GLuint depthCubemap, const std::string& fileName) {
    GLint prevFramebuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);

    GLuint readFBO;
    glGenFramebuffers(1, &readFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, readFBO);

    int width = 0;
    int height = 0;
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);

    std::vector<float> depthData(width * height * 6);

    GLenum cubeMapFaces[6] = {
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    for (int face = 0; face < 6; ++face) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubeMapFaces[face], depthCubemap, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer is not complete!" << std::endl;
            glBindFramebuffer(GL_FRAMEBUFFER, prevFramebuffer);
            glDeleteFramebuffers(1, &readFBO);
            return;
        }
        glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, &depthData[width * height * face]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, prevFramebuffer);
    glDeleteFramebuffers(1, &readFBO);

    std::vector<unsigned char> imageData(width * height * 6);
    for (int i = 0; i < width * height * 6; ++i) {
        imageData[i] = static_cast<unsigned char>(depthData[i] * 255.0f);
    }

    std::vector<unsigned char> finalImage(width * 3 * height * 2);
    for (int face = 0; face < 6; ++face) {
        int xOffset = (face % 3) * width;
        int yOffset = (face / 3) * height;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                finalImage[(yOffset + y) * (width * 3) + xOffset + x] = imageData[face * width * height + y * width + x];
            }
        }
    }

    stbi_write_png(fileName.c_str(), width * 3, height * 2, 1, finalImage.data(), width * 3);
}

void saveColorCubemapToImage(GLuint colorCubemap, const std::string& fileName) {
    GLint prevFramebuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);

    GLuint readFBO;
    glGenFramebuffers(1, &readFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, readFBO);

    int width = 0;
    int height = 0;
    glBindTexture(GL_TEXTURE_CUBE_MAP, colorCubemap);

    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);

    if (width == 0 || height == 0) {
        std::cerr << "Invalid cubemap dimensions." << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, prevFramebuffer);
        glDeleteFramebuffers(1, &readFBO);
        return;
    }

    std::vector<unsigned char> colorData(width * height * 3 * 6);

    GLenum cubeMapFaces[6] = {
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    for (int face = 0; face < 6; ++face) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cubeMapFaces[face], colorCubemap, 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer is not complete for face " << face << ": " << status << std::endl;
            glBindFramebuffer(GL_FRAMEBUFFER, prevFramebuffer);
            glDeleteFramebuffers(1, &readFBO);
            return;
        }
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, &colorData[width * height * 3 * face]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, prevFramebuffer);
    glDeleteFramebuffers(1, &readFBO);

    std::vector<unsigned char> finalImage(width * 3 * height * 2 * 3); // RGB output, so we multiply by 3.
    for (int face = 0; face < 6; ++face) {
        int xOffset = (face % 3) * width;
        int yOffset = (face / 3) * height;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                for (int c = 0; c < 3; ++c) { // Copy RGB channels
                    finalImage[((yOffset + y) * (width * 3) + (xOffset + x)) * 3 + c] =
                            colorData[(face * width * height + y * width + x) * 3 + c];
                }
            }
        }
    }

    if (!stbi_write_png(fileName.c_str(), width * 3, height * 2, 3, finalImage.data(), width * 3 * 3)) {
        std::cerr << "Failed to write PNG file." << std::endl;
    }
}

void printMatrix(const glm::mat4& matrix) {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            std::cout << std::setw(10) << std::fixed << std::setprecision(4) << matrix[row][col] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl; // Separate matrices with a blank line
}

// Function to print all matrices in a vector
void printMatrixVector(const std::vector<glm::mat4>& matrices) {
    for (size_t i = 0; i < matrices.size(); ++i) {
        std::cout << "Matrix " << i + 1 << ":\n";
        printMatrix(matrices[i]);
    }
}


bool loadFbx(const std::string& fbxFilePath,
             std::vector<GLfloat>& vertices,
             std::vector<GLfloat>& normals,
             std::vector<GLfloat>& uvs,
             std::vector<GLuint>& indices,
             std::vector<GLfloat>& colors) {
    // Create an instance of the Assimp Importer
    Assimp::Importer importer;

    // Read the file
    const aiScene* scene = importer.ReadFile(fbxFilePath,
                                             aiProcess_Triangulate |       // Triangulate all faces
                                             aiProcess_GenSmoothNormals | // Generate smooth normals if not present
                                             aiProcess_FlipUVs);

    // Check if the file was successfully loaded
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return false;
    }


    // Iterate through meshes in the scene
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];


        // Process vertices
        for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
            // Vertex position
            vertices.push_back(mesh->mVertices[v].x);
            vertices.push_back(mesh->mVertices[v].y);
            vertices.push_back(mesh->mVertices[v].z);

            // Vertex normals
            if (mesh->HasNormals()) {
                normals.push_back(mesh->mNormals[v].x);
                normals.push_back(mesh->mNormals[v].y);
                normals.push_back(mesh->mNormals[v].z);
            }

            // Texture coordinates (UVs)
            if (mesh->mTextureCoords[0]) { // Assimp supports up to 8 UV channels; we use the first
                uvs.push_back(mesh->mTextureCoords[0][v].x);
                uvs.push_back(mesh->mTextureCoords[0][v].y);
            } else {
                uvs.push_back(0.0f); // Default value if no UVs are present
                uvs.push_back(0.0f);
            }

            // Vertex colors
            if (mesh->HasVertexColors(0)) { // Assimp supports up to 8 color channels; we use the first
                colors.push_back(mesh->mColors[0][v].r);
                colors.push_back(mesh->mColors[0][v].g);
                colors.push_back(mesh->mColors[0][v].b);
            } else {
                colors.push_back(1.0f); // Default color (white)
                colors.push_back(1.0f);
                colors.push_back(1.0f);
            }
        }

        // Process indices
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
            aiFace face = mesh->mFaces[f];
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }
    }

    return true;
}

bool loadFbx(const std::string& fbxFilePath,
             std::vector<GLfloat>& vertices,
             std::vector<GLfloat>& normals,
             std::vector<GLfloat>& uvs,
             std::vector<GLuint>& indices,
             std::vector<GLfloat>& colors,
             std::vector<std::string>& texturePaths) {
    // Create an instance of the Assimp Importer
    Assimp::Importer importer;

    // Read the file
    const aiScene* scene = importer.ReadFile(fbxFilePath,
                                             aiProcess_Triangulate |       // Triangulate all faces
                                             aiProcess_GenSmoothNormals | // Generate smooth normals if not present
                                             aiProcess_FlipUVs);

    // Check if the file was successfully loaded
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return false;
    }

    bool texturesFound = false;

    // Iterate through meshes in the scene
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];

        // Process vertices
        for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
            // Vertex position
            vertices.push_back(mesh->mVertices[v].x);
            vertices.push_back(mesh->mVertices[v].y);
            vertices.push_back(mesh->mVertices[v].z);

            // Vertex normals
            if (mesh->HasNormals()) {
                normals.push_back(mesh->mNormals[v].x);
                normals.push_back(mesh->mNormals[v].y);
                normals.push_back(mesh->mNormals[v].z);
            }

            // Texture coordinates (UVs)
            if (mesh->mTextureCoords[0]) { // Assimp supports up to 8 UV channels; we use the first
                uvs.push_back(mesh->mTextureCoords[0][v].x);
                uvs.push_back(mesh->mTextureCoords[0][v].y);
            } else {
                uvs.push_back(0.0f); // Default value if no UVs are present
                uvs.push_back(0.0f);
            }

            // Vertex colors
            if (mesh->HasVertexColors(0)) { // Assimp supports up to 8 color channels; we use the first
                colors.push_back(mesh->mColors[0][v].r);
                colors.push_back(mesh->mColors[0][v].g);
                colors.push_back(mesh->mColors[0][v].b);
            } else {
                colors.push_back(1.0f); // Default color (white)
                colors.push_back(1.0f);
                colors.push_back(1.0f);
            }
        }

        // Process indices
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
            aiFace face = mesh->mFaces[f];
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }
    }

    // Process textures
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        aiMaterial* material = scene->mMaterials[i];

        for (unsigned int t = 0; t < material->GetTextureCount(aiTextureType_DIFFUSE); ++t) {
            aiString path;
            if (material->GetTexture(aiTextureType_DIFFUSE, t, &path) == AI_SUCCESS) {
                texturePaths.push_back(path.C_Str());
                texturesFound = true;
            }
        }
    }

    if (!texturesFound) {
        std::cout << "No texture data found in the FBX file." << std::endl;
    }

    return true;
}


AnimationData buildBoneHierarchy(const aiScene* scene, FileAnimationData& fileAnimationData) {
    AnimationData hierarchicalData;
    // Map bone names to their indices in the bone list
    hierarchicalData.bones.resize(fileAnimationData.boneMap.size());

    // Map bone names to their indices in the bone list
    std::unordered_map<std::string, int> boneNameToIndex;
    for (const auto& bonePair : fileAnimationData.boneMap) {
        const std::string& boneName = bonePair.first;
        const BoneInfo& boneInfo = bonePair.second;

        BoneNode boneNode;
        boneNode.index = boneInfo.index;
        boneNode.name = boneName;
        boneNode.offsetMatrix = boneInfo.offsetMatrix;
        boneNode.localTransform = glm::mat4(1.0f); // Default identity matrix

        // Directly assign the bone node into the vector at the specified index
        hierarchicalData.bones[boneNode.index] = boneNode;
        boneNameToIndex[boneName] = boneInfo.index;
    }

    // Recursively process the scene graph to establish parent-child relationships
    std::function<void(const aiNode*, int, int*)> processNode = [&](const aiNode* node, int parentIndex, int * rootNode) {
        std::string nodeName(node->mName.C_Str());

        // Check if this node corresponds to a bone
        auto it = boneNameToIndex.find(nodeName);
        if (it != boneNameToIndex.end()) {

            int boneIndex = it->second;
            if (*rootNode == -1){
                *rootNode = boneIndex;
            }
            if (parentIndex != -1) {
                hierarchicalData.bones[parentIndex].childrenIndices.push_back(boneIndex);
            }

            // Set local transform (from aiNode transformation)
            aiMatrix4x4 aiTransform = node->mTransformation;
            glm::mat4 localTransform = glm::transpose(glm::make_mat4(&aiTransform.a1));
            hierarchicalData.bones[boneIndex].localTransform = localTransform;

            // Recursively process children
            for (unsigned int i = 0; i < node->mNumChildren; i++) {
                processNode(node->mChildren[i], boneIndex, rootNode);
            }
        } else {
            // Node is not a bone, process children but skip storing it
            for (unsigned int i = 0; i < node->mNumChildren; i++) {
                processNode(node->mChildren[i], parentIndex, rootNode);
            }
        }
    };

    int rootNode = -1;
    // Start recursion from the root node
    processNode(scene->mRootNode, -1, &rootNode);
    hierarchicalData.rootIndex = rootNode;

    // Copy vertices, indices, and animations
    hierarchicalData.vertices = std::move(fileAnimationData.vertices);
    hierarchicalData.indices = std::move(fileAnimationData.indices);
    hierarchicalData.animations = std::move(fileAnimationData.animations);

    return hierarchicalData;
}

AnimationData loadFBXAnimation(const std::string& filePath) {
    FileAnimationData FileanimationData;

    // Import FBX file
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_LimitBoneWeights);

    if (!scene || !scene->mRootNode) {
        throw std::runtime_error("Failed to load file: " + filePath + " (" + importer.GetErrorString() + ")");
    }

    // 1. Process Mesh Data
    if (scene->mNumMeshes > 0) {
        const aiMesh* mesh = scene->mMeshes[0];
        FileanimationData.vertices.reserve(mesh->mNumVertices);

        std::unordered_map<std::string, int> boneMapping; // Bone name -> ID
        std::vector<int> boneWeightsPerVertex(mesh->mNumVertices, 0);

        // Extract vertex data
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

            if (mesh->mTextureCoords[0]) {
                vertex.uv = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            } else {
                vertex.uv = glm::vec2(0.0f, 0.0f);
            }

            vertex.boneWeights = glm::vec4(-2.0f);
            vertex.boneIDs = glm::ivec4(-1);

            FileanimationData.vertices.push_back(vertex);
        }

        // Extract bone data
// Extract bone data
        for (unsigned int i = 0; i < mesh->mNumBones; i++) {
            std::string boneName = mesh->mBones[i]->mName.C_Str();
            if (boneMapping.find(boneName) == boneMapping.end()) {
                int boneID = static_cast<int>(boneMapping.size());
                boneMapping[boneName] = boneID;

                BoneInfo boneInfo;

                // Ensure the offset matrix is handled correctly
                // Assimp stores matrices in column-major order, but GLM expects row-major matrices.
                // glm::transpose ensures the conversion between these formats.
                boneInfo.offsetMatrix = glm::transpose(glm::make_mat4(&mesh->mBones[i]->mOffsetMatrix.a1));
                boneInfo.index = boneID;


                FileanimationData.boneMap[boneName] = boneInfo;
            }

            int boneID = boneMapping[boneName];

            // Assign weights to vertices
            for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
                unsigned int vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
                float weight = mesh->mBones[i]->mWeights[j].mWeight;

                // Ensure we don't exceed 4 bone influences per vertex
                for (int k = 0; k < 4; ++k) {
                    if (FileanimationData.vertices[vertexID].boneWeights[k] < 0.0f) { // Unused slot
                        FileanimationData.vertices[vertexID].boneWeights[k] = weight;
                        FileanimationData.vertices[vertexID].boneIDs[k] = boneID;
                        break;
                    }
                }
            }
        }

        // Extract indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                FileanimationData.indices.push_back(face.mIndices[j]);
            }
        }
    }

    // 2. Process Animation Data
    if (scene->mNumAnimations > 0) {
        for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
            const aiAnimation* anim = scene->mAnimations[i];
            AnimationTrack animationTrack;
            for (unsigned int j = 0; j < anim->mNumChannels; j++) {
                const aiNodeAnim* channel = anim->mChannels[j];
                BoneAnimation boneAnim;
                boneAnim.name = channel->mNodeName.C_Str();

                for (unsigned int k = 0; k < channel->mNumPositionKeys; k++) {
                    aiVectorKey posKey = channel->mPositionKeys[k];
                    aiQuatKey rotKey = channel->mRotationKeys[k];
                    aiVectorKey scaleKey = channel->mScalingKeys[k];

                    Keyframe keyframe;
                    keyframe.position = glm::vec3(posKey.mValue.x, posKey.mValue.y, posKey.mValue.z);
                    keyframe.rotation = glm::quat(rotKey.mValue.w, rotKey.mValue.x, rotKey.mValue.y, rotKey.mValue.z);
                    keyframe.scale = glm::vec3(scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z);

                    boneAnim.keyframeTimes.push_back(static_cast<float>((float)posKey.mTime/(float)anim->mTicksPerSecond));
                    boneAnim.keyframes.push_back(keyframe);
                }
                animationTrack.boneAnims.push_back(boneAnim);

            }
            FileanimationData.animations.push_back(animationTrack);
        }
    }

    return buildBoneHierarchy(scene, FileanimationData);
}


// Function to create and configure the G-buffer
void createGBuffer(GLuint* gBuffer, GLuint* gColour, GLuint* gPosition, GLuint* gNormal, GLuint* rboDepth, GLuint* emit, int width, int height) {

    // Initialize FBO and corresponding textures
    glGenFramebuffers(1, gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, *gBuffer);

    // Color texture
    glGenTextures(1, gColour);
    glBindTexture(GL_TEXTURE_2D, *gColour);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *gColour, 0);

    // Position texture
    glGenTextures(1, gPosition);
    glBindTexture(GL_TEXTURE_2D, *gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, *gPosition, 0);

    // Normal texture
    glGenTextures(1, gNormal);
    glBindTexture(GL_TEXTURE_2D, *gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, *gNormal, 0);

    // Emissive texture
    glGenTextures(1, emit);
    glBindTexture(GL_TEXTURE_2D, *emit);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, *emit, 0);

    // Specify multiple render targets
    GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);

    // Depth renderbuffer
    glGenRenderbuffers(1, rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, *rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *rboDepth);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void saveDepthTexture(GLuint fbo, std::string filename) {
    // Bind the framebuffer to read its texture
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Get the width and height of the texture attached to the framebuffer
    GLuint attachedTexture;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, (GLint*)&attachedTexture);

    if (attachedTexture == 0) {
        std::cerr << "Error: No depth texture attached to the framebuffer!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Query texture width and height
    GLint width, height;
    glBindTexture(GL_TEXTURE_2D, attachedTexture);  // Ensure we're querying the correct texture
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // If the texture is cube map, you may need to adjust the binding to query faces (assuming you want depth from a single face)
    // In that case, you would need to check the attachment type and query the texture accordingly.

    int channels = 3;

    std::vector<float> depth(width * height);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glReadBuffer(GL_DEPTH_COMPONENT);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::vector<unsigned char> img(width * height * 3);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            // Invert the row we're looking at so image is right way up
            float depthValue = depth[j + ((height - 1 - i) * width)];

            // Invert depth for better contrast (depths closer to 1 are stretched)
            float scaledDepth = 1.0 - pow((1.0f - depthValue), 0.5);

            unsigned char intensity = static_cast<unsigned char>(scaledDepth * 255);

            img[3 * (j + (i * width))] = intensity;
            img[3 * (j + (i * width)) + 1] = intensity;
            img[3 * (j + (i * width)) + 2] = intensity;
        }
    }

    int rc = stbi_write_png(filename.c_str(), width, height, channels, img.data(), width * channels);
    std::cout << "rc=" << rc << std::endl;
}
