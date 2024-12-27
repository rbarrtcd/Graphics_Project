#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <render/shader.h>
#include "animationData.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "glm/gtc/type_ptr.hpp"

#include "entity.h"
#include "light.h"
#include "utilities.h"
#include "geometry.h"
#include "primitives.h"
#include "skybox.h"
#include "deferredShader.h"

#include <vector>
#include <iostream>
#include <thread>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>
#include <functional>
#include <iomanip>
#include <xutility>

static GLFWwindow *window;
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// OpenGL camera view parameters
static glm::vec3 eye_center;
static glm::vec3 lookat(0, 0, 0);
static glm::vec3 up(0, 1, 0);

// View control
static float viewAzimuth = 0.f;
static float viewPolar = 0.f;
static float viewDistance = 800.0f;



static glm::vec3 cameraSpeed = glm::vec3(250, 250, 250);
float yaw = 180.0f;
float pitch = -45.0f;
float mouseSensitivity = 0.05f;
float lastX, lastY;


const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(205.0f, 0.0f, 0.0f);
static glm::vec3 lightIntensity = 1.0f * ((8.0f * wave500) + (15.6f * wave600) + (18.4f * wave700));
static glm::vec3 lightPosition(-275.0f, 500.0f, -275.0f);






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





void camera_update(float deltaTime){
    glm::vec3 forward = glm::normalize(lookat - eye_center);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    glm::vec3 current_speed = cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        current_speed *= glm::vec3(3.0);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        eye_center += forward * current_speed*deltaTime;
        lookat += forward * current_speed*deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        eye_center -= forward * current_speed*deltaTime;
        lookat -= forward * current_speed*deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        eye_center -= right * current_speed*deltaTime;
        lookat -= right * current_speed*deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        eye_center += right * current_speed*deltaTime;
        lookat += right * current_speed*deltaTime;
    }
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
                    /*
                    std::cout << "Animation: " << anim->mName.C_Str() << std::endl;
                    std::cout << "keyframe: " << (float)posKey.mTime << std::endl;
                    std::cout << "Duration (ticks): " << anim->mDuration << std::endl;
                    std::cout << "Ticks per second: " << anim->mTicksPerSecond << std::endl;
                    std::cout << "Stored time: " <<  boneAnim.keyframeTimes.back() << std::endl;
                    if (anim->mTicksPerSecond == 0) {
                        std::cout << "Keyframe times are in seconds." << std::endl;
                    } else {
                        std::cout << "Keyframe times are in ticks. Convert to seconds using: time = mTime / mTicksPerSecond." << std::endl;
                    }
                     */
                }
                animationTrack.boneAnims.push_back(boneAnim);

            }
            FileanimationData.animations.push_back(animationTrack);
        }
    }

    return buildBoneHierarchy(scene, FileanimationData);
}



bool pause = false;





int main(void)
{
    // Initialise GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int const screenWidth = 1920;
    int const screenHeight = 1080;

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(screenWidth, screenHeight, "Graphics Lab", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to open a GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
    {
        std::cerr << "Failed to initialize OpenGL context." << std::endl;
        return -1;
    }


    GLuint geometry_programID = 0;
    if (geometry_programID == 0)
    {
        geometry_programID = LoadShadersFromFile("../shaders/geometry.vert", "../shaders/geometry.frag");
        if (geometry_programID == 0)
        {
            std::cerr << "Failed to load shaders." << std::endl;
        }
    }

    GLuint lightingPass_shader = 0;
    if (lightingPass_shader == 0)
    {
        lightingPass_shader = LoadShadersFromFile("../shaders/lighting.vert", "../shaders/lighting.frag");
        if (lightingPass_shader == 0)
        {
            std::cerr << "Failed to load shaders." << std::endl;
        }
    }

    GLuint shadowMap_shader = 0;
    if (shadowMap_shader == 0)
    {
        shadowMap_shader = LoadShadersFromFile("../shaders/shadowMap.vert", "../shaders/shadowMap.geom", "../shaders/shadowMap.frag");
        if (shadowMap_shader == 0)
        {
            std::cerr << "Failed to load shaders." << std::endl;
        }
    }


    GLuint skybox_shader = 0;
    if (skybox_shader == 0)
    {
        skybox_shader = LoadShadersFromFile("../shaders/skybox.vert", "../shaders/skybox.frag");
        if (skybox_shader == 0)
        {
            std::cerr << "Failed to load shaders." << std::endl;
        }
    }

    GLuint entity_shader = 0;
    if (entity_shader == 0)
    {
        entity_shader = LoadShadersFromFile("../shaders/entity.vert", "../shaders/entity.frag");
        if (entity_shader == 0)
        {
            std::cerr << "Failed to load shaders." << std::endl;
        }
    }

    GLuint animation_shader = 0;
    if (animation_shader == 0)
    {
        animation_shader = LoadShadersFromFile("../shaders/entityAnimated.vert", "../shaders/entityAnimated.frag");
        if (animation_shader == 0)
        {
            std::cerr << "Failed to load shaders." << std::endl;
        }
    }

    GLuint entityLight_shader = 0;
    if (entityLight_shader == 0)
    {
        entityLight_shader = LoadShadersFromFile("../shaders/shadowEntity.vert", "../shaders/shadowEntity.geom", "../shaders/shadowEntity.frag");
        if (entityLight_shader == 0)
        {
            std::cerr << "Failed to load shaders." << std::endl;
        }
    }

    GLuint gBuffer, gColour, gPosition, gNormal, rboDepth, gEmit;


    // Create and configure the G-buffer
    createGBuffer(&gBuffer, &gColour, &gPosition, &gNormal, &rboDepth, &gEmit, screenWidth, screenHeight);


    // Background
    glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);


    DeferredShader * deferredShader = new DeferredShader(lightingPass_shader);
    deferredShader->initialize();
    std::vector<Geometry*> geometries;
    std::vector<Entity*> entities;
    std::vector<Light*> lights;


    const char* concrete_texture = "../assets/textures/concrete/gravel_concrete_03_diff_4k.jpg";

    Geometry * groundPlane = new Geometry(glm::vec3(0,0,0), glm::vec3(640, 1, 640), glm::vec3(0,0,0), geometry_programID);
    groundPlane->initialize(meshData_plane,concrete_texture);
    geometries.push_back(groundPlane);

    Geometry * wall = new Geometry(glm::vec3(0,0,-320), glm::vec3(480, 1, 640), glm::vec3(90,0,0), geometry_programID);
    wall->initialize(meshData_plane,concrete_texture);
    geometries.push_back(wall);

    Geometry * highFloor = new Geometry(glm::vec3(0,320,-560), glm::vec3(480, 1, 480), glm::vec3(0,0,0), geometry_programID);
    highFloor->initialize(meshData_plane,concrete_texture);
    geometries.push_back(highFloor);

    Geometry * wall2 = new Geometry(glm::vec3(0,0,-800), glm::vec3(640, 1, 640), glm::vec3(90,180,0), geometry_programID);
    wall2->initialize(meshData_plane,concrete_texture);
    geometries.push_back(wall2);

    Geometry * testBox1 = new Geometry(glm::vec3(128,32,0), glm::vec3(64, 64, 64), glm::vec3(0,45,0), geometry_programID);
    testBox1->initialize(meshData_box, concrete_texture);
    geometries.push_back(testBox1);

    Geometry * testBox2 = new Geometry(glm::vec3(0,80,-160), glm::vec3(32, 64, 48), glm::vec3(45,45,0), geometry_programID);
    testBox2->initialize(meshData_box, concrete_texture);
    geometries.push_back(testBox2);

    std::vector<GLfloat> vertices, normals, uvs, colours;
    std::vector<GLuint> indices;
    loadFbx("../assets/models/lowPolyHuman/ManUnity.fbx", vertices, normals, uvs, indices, colours);
    //const std::string objFilePath = "../assets/models/lowPolyHuman/Man.obj";
    //MeshData mesh = loadObjVertexData(objFilePath);
    //for (int i = 0; i < vertices.size(); ++i) {
    //    colors.push_back(1.0);
    //
    //}
    Entity * testModel = new Entity(glm::vec3(0,0,0), glm::vec3(10, 10, 10), glm::vec3(0,90,90), entity_shader, animation_shader);
    testModel->setTexture("../assets/models/lowPolyHuman/ManColors.png");
    testModel->loadModelData(vertices, normals, colours, indices, uvs);
    AnimationData testAnimData = loadFBXAnimation("../assets/models/lowPolyHuman/ManUnity.fbx");
    //printMatrix(testAnimData.bones[testAnimData.rootIndex].offsetMatrix);
    testModel->loadAnimationData(testAnimData);
    entities.push_back(testModel);



    lightPosition = glm::vec3(150, 128, -280);
    Light * testLight = new Light(lightPosition, glm::vec3(255, 164.65, 38.43), 100.0, 1024, 4096.0);
    lights.push_back(testLight);

    //lightPosition = glm::vec3(-150, 64, -100);
    Light * testLight2 = new Light(glm::vec3(-150, 64, -100), glm::vec3(123, 244.65, 38.43), 50.0, 960, 2048.0);
    lights.push_back(testLight2);


    Light * sun = new Light(glm::vec3(100, 500, -600), glm::vec3(255, 164.65, 38.43), 1000.0, 1000, 2400.0);
    lights.push_back(sun);

    Geometry * lightBox = new Geometry(lightPosition, glm::vec3(4, 4, 4), glm::vec3(0,0,0), geometry_programID);
    lightBox->initialize(meshData_box, concrete_texture);
    geometries.push_back(lightBox);

    // Camera setup
    eye_center.y = viewDistance * cos(viewPolar);
    eye_center.x = viewDistance * cos(viewAzimuth);
    eye_center.z = viewDistance * sin(viewAzimuth);

    glm::mat4 viewMatrix, projectionMatrix;
    glm::float32 FoV = 70;
    glm::float32 zNear = 0.4f;
    glm::float32 zFar = 3000.0f;
    projectionMatrix = glm::perspective(glm::radians(FoV), (float) screenWidth / (float) screenHeight, zNear, zFar);

    Skybox * theSkybox = new Skybox();
    const std::vector<std::string> dayFilePaths = {"../assets/skybox/daytime/day_px.png",
                                                   "../assets/skybox/daytime/day_nx.png",
                                                   "../assets/skybox/daytime/day_py.png",
                                                   "../assets/skybox/daytime/day_ny.png",
                                                   "../assets/skybox/daytime/day_pz.png",
                                                   "../assets/skybox/daytime/day_nz.png"};

    const std::vector<std::string> nightFilePaths = {"../assets/skybox/night/px.png",
                                                   "../assets/skybox/night/nx.png",
                                                   "../assets/skybox/night/py.png",
                                                   "../assets/skybox/night/ny.png",
                                                   "../assets/skybox/night/pz.png",
                                                   "../assets/skybox/night/nz.png"};

    theSkybox->initialise(skybox_shader);
    int skyboxDay = theSkybox->addSkybox(dayFilePaths);
    int skyboxNight = theSkybox->addSkybox(nightFilePaths);

    static double lastTime = glfwGetTime();
    float time = 0.0f;			// Animation time
    float fTime = 0.0f;			// Time for measuring fps
    unsigned long frames = 0;

    float lastFrameTime = 0.0f;
    do
    {
        glfwPollEvents();
        lightBox->position = lightPosition;
        lightBox->modelMatrix = computeModelMatrix(lightBox->position,  lightBox->rotation, lightBox->scale);
        testLight->position = lightPosition;
        testLight->update();

        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
            testLight->intensity = testLight->intensity * 1.01;
            testModel->playAnimation(0);
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            testLight->intensity = testLight->intensity / 1.01;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            theSkybox->skyboxIndex = skyboxDay;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            theSkybox->skyboxIndex = skyboxNight;
        }

        float currentFrameTime = glfwGetTime();
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        double currentTime = glfwGetTime();
        float ModeldeltaTime = float(currentTime - lastTime);
        lastTime = currentTime;
        time += deltaTime;

        camera_update(deltaTime);

        viewMatrix = glm::lookAt(eye_center, lookat, up);
        glm::mat4 vp = projectionMatrix * viewMatrix;



        //Render to buffer
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        theSkybox->render(viewMatrix, projectionMatrix);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




        for (Geometry * g:geometries) {
            g->render(vp);
        }


        for (Entity * e:entities) {
            e->update();
            e->render(vp);
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            //For debugging purposed
            //saveCubeMapDepthTexture(testLight->shadowFBO, testLight->shadowCubeMap, "depther.png");
            saveGBufferTextures(gBuffer, gColour, gPosition, gNormal, rboDepth, gEmit, 1920, 1080);
            //saveColorCubemapToImage(theSkybox->textureIDs[theSkybox->skyboxIndex], "skybox_texture.png");
            saveDepthCubemapToImage(sun->shadowCubeMap, "light_depth_map.png");
            //saveDepthCubemapToImage(testLight2->shadowCubeMap, "light_depth_map2.png");
        }

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.2,9.0);
        glm::mat4 lightViewMatrix;
        //Shadow Mapping Pass
        for (Light* l : lights) {
            glBindFramebuffer(GL_FRAMEBUFFER, l->shadowFBO);
            glViewport(0, 0, l->shadowMapSize, l->shadowMapSize);
            glClear(GL_DEPTH_BUFFER_BIT);

            // Render the scene geometry
            for (Geometry *g: geometries) {
                g->lightRender(shadowMap_shader, *l);
            }

            for (Entity * e:entities) {
                e->lightRender(shadowMap_shader, entityLight_shader, *l);
            }
        }
        glDisable(GL_POLYGON_OFFSET_FILL);



        /**/
        //Render to Screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, screenWidth, screenHeight);



        //Implmenet Lighting Pass
        deferredShader->render(gBuffer, gColour, gPosition, gNormal, gEmit, lights);
        glfwSwapBuffers(window);
        if (pause){
            std::this_thread::sleep_for(std::chrono::seconds(10));
            pause = false;
        }
                 /**/

        /*
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, screenWidth, screenHeight);
        glUseProgram(skybox_shader);
        GLuint skybox_texture = testLight->shadowCubeMap;
        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(viewMatrix));
        // Set shader uniforms
        GLuint viewLoc = glGetUniformLocation(skybox_shader, "view");
        GLuint projectionLoc = glGetUniformLocation(skybox_shader, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewNoTranslation));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));


// Bind the skybox texture to the appropriate texture unit
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
        GLuint textureLoc = glGetUniformLocation(skybox_shader, "skybox");
        glUniform1i(textureLoc, 0);

        float skyboxVertices[] = {
                // positions
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                -1.0f,  1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f,  1.0f
        };

        GLuint skyboxVAO, skyboxVBO;
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);

        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindVertexArray(0);


// Render the cube representing the skybox
        glBindVertexArray(skyboxVAO);
        glDepthMask(GL_FALSE); // Disable depth writing
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);  // Re-enable depth writing
*/


    }
    while (!glfwWindowShouldClose(window));

    // Clean up
    for (Geometry * g:geometries) {
        g->cleanup();
    }


    glDeleteFramebuffers(1, &gBuffer);
    glDeleteTextures(1, &gPosition);
    glDeleteTextures(1, &gNormal);
    glDeleteTextures(1, &gColour);
    glDeleteRenderbuffers(1, &rboDepth);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true;

    // Initialize the lastX and lastY values if this is the first mouse movement
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // Calculate offset from last mouse position
    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    // Apply sensitivity to the offsets
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    // Update yaw and pitch
    yaw += xOffset;
    pitch += yOffset;

    // Constrain pitch to prevent flipping
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Calculate the new `lookat` direction
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    lookat = glm::normalize(direction) + eye_center; // Update `lookat` based on camera position
}



// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        viewAzimuth = 0.f;
        viewPolar = 0.f;
        eye_center.y = viewDistance * cos(viewPolar);
        eye_center.x = viewDistance * cos(viewAzimuth);
        eye_center.z = viewDistance * sin(viewAzimuth);
        std::cout << "Reset." << std::endl;
    }

    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        lightPosition.x = lightPosition.x + 8;

    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        lightPosition.x = lightPosition.x - 8;

    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        lightPosition.z = lightPosition.z + 8;
    }

    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        lightPosition.z = lightPosition.z - 8;

    if (key == GLFW_KEY_I && action == GLFW_PRESS)
        lightPosition.y = lightPosition.y + 8;

    if (key == GLFW_KEY_K && action == GLFW_PRESS)
        lightPosition.y = lightPosition.y - 8;


    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}
