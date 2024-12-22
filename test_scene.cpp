#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <render/shader.h>
#include "test_scene.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "glm/gtc/type_ptr.hpp"


#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/gl.h>
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
float yaw = -90.0f;
float pitch = 0.0f;
float mouseSensitivity = 0.05f;
float lastX, lastY;


const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(205.0f, 0.0f, 0.0f);
static glm::vec3 lightIntensity = 1.0f * ((8.0f * wave500) + (15.6f * wave600) + (18.4f * wave700));
static glm::vec3 lightPosition(-275.0f, 500.0f, -275.0f);






void saveGBufferTextures(GLuint gBuffer, GLuint gColour, GLuint gPosition, GLuint gNormal, GLuint rboDepth, int width, int height) {
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // Allocate space for texture data
    unsigned char* colourData = new unsigned char[width * height * 4]; // RGBA
    float* positionData = new float[width * height * 3]; // XYZ as floats
    float* normalData = new float[width * height * 3]; // XYZ as floats
    float * depthData = new float[width * height]; // Depth as grayscale (0-255)

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
    std::cout << "depth[0] = " << (float) depthData[0] << std::endl;
    // Save depth as PNG (normalize to 0-255 range)
    unsigned char* depthNormalized = new unsigned char[width * height];
    for (int i = 0; i < width * height; ++i) {
        depthNormalized[i] = (unsigned char)(depthData[i] * 255); // Depth range (0-1) to (0-255)
    }
    stbi_write_png("gBuffer_depth.png", width, height, 1, depthNormalized, width);

    // Clean up
    delete[] colourData;
    delete[] positionData;
    delete[] normalData;
    delete[] depthData;
    delete[] positionNormalized;
    delete[] normalNormalized;
    delete[] depthNormalized;

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind framebuffer
}

static void saveDepthTexture(GLuint fbo, std::string filename) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Get the attachment type (e.g., texture or renderbuffer)
    GLint attachmentType;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &attachmentType);

    int width = 0, height = 0;

    if (attachmentType == GL_TEXTURE) {
        // If it's a texture, query its size
        GLint textureID;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
        glBindTexture(GL_TEXTURE_2D, 0);
    } else if (attachmentType == GL_RENDERBUFFER) {
        // If it's a renderbuffer, query its size
        GLint renderbufferID;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &renderbufferID);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbufferID);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    // Fallback to default size if dimensions are not set
    if (width == 0 || height == 0) {
        width = 1920;
        height = 1080;
    }

    int channels = 3;

    std::vector<float> depth(width * height);
    glReadBuffer(GL_DEPTH_COMPONENT);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::vector<unsigned char> img(width * height * 3);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            //Invert the row we're looking at so image is right way up

            float depthValue = depth[j + ((height - 1 - i) * width)];

            // Invert depth for better contrast (depths closer to 1 are stretched)
            //invert, sqaureroot, invert again
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

GLuint createDepthCubeMapFramebuffer(int resolution, GLuint* depthCubeMap) {
    GLuint fbo;

    // Generate and bind the framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Generate the cube map texture
    glGenTextures(1, depthCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *depthCubeMap);


    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X , 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X , 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y , 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y , 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z , 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z , 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);


    // Set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Attach the cube map texture to the framebuffer as the depth attachment
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *depthCubeMap, 0);

    // Disable color buffer outputs since this is a depth-only framebuffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 0;
    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fbo;
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


class Light {
public:
    glm::vec3 position;
    glm::vec3 colour;
    float intensity;
    float lightRange;
    int shadowMapSize;

    GLuint shadowFBO;
    GLuint shadowCubeMap;

    glm::mat4 lightProjectionMatrix;

    glm::mat4 VPmatrices[6];

    Light(glm::vec3 position, glm::vec3 colour, float intensity, int shadowMapSize, float Range) {
        this->position = position;
        this->colour = colour;
        this->intensity = intensity;
        this->shadowMapSize = shadowMapSize;
        this->lightRange = Range;

        // Create the depth cube map and framebuffer
        shadowFBO = createDepthCubeMapFramebuffer(shadowMapSize, &shadowCubeMap);

        update();
    }

    void update(){
        calculateVPMatrices();
    }

    void calculateVPMatrices(){
        lightProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, lightRange);
        glm::vec3 directions[6] = {
                glm::vec3(1.0f, 0.0f, 0.0f),   // +X
                glm::vec3(-1.0f, 0.0f, 0.0f),  // -X
                glm::vec3(0.0f, 1.0f, 0.0f),   // +Y
                glm::vec3(0.0f, -1.0f, 0.0f),  // -Y
                glm::vec3(0.0f, 0.0f, 1.0f),   // +Z
                glm::vec3(0.0f, 0.0f, -1.0f)   // -Z
        };

        glm::vec3 upVectors[6] = {
                glm::vec3(0.0f, -1.0f, 0.0f),  // +X
                glm::vec3(0.0f, -1.0f, 0.0f),  // -X
                glm::vec3(0.0f, 0.0f, 1.0f),   // +Y
                glm::vec3(0.0f, 0.0f, -1.0f),  // -Y
                glm::vec3(0.0f, -1.0f, 0.0f),  // +Z
                glm::vec3(0.0f, -1.0f, 0.0f)   // -Z
        };

        for (int i = 0; i < 6; ++i) {
            VPmatrices[i] = lightProjectionMatrix * glm::lookAt(position,position + directions[i],upVectors[i]);
        }

    }
};



class Geometry {
public:
    // Constructor
    Geometry(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, GLuint programID)
            : position(position), scale(scale), rotation(rotation), programID(programID){

    }

    virtual ~Geometry() = default;

    // Virtual functions to be overridden by derived classes
    //virtual void initialize(const char* texture_file_path) = 0;
    virtual void render(glm::mat4 cameraMatrix) = 0;
    virtual void cleanup() = 0;
    virtual void lightRender(GLuint programID, Light light) = 0;

protected:
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
    GLuint programID;
};

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

class Entity {
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

    GLuint staticID;
    GLuint animatedID;

    // Shader variable IDs
    GLuint mvpMatrixID;
    GLuint textureSamplerID;
    GLuint modelMatrixID;
    //static GLuint programID;
    glm::mat4 modelMatrix;


    GLuint AnimatedmvpMatrixID;
    GLuint AnimatedmodelMatrixID;
    GLuint AnimatedvertexArrayID;
    GLuint AnimatedvertexBufferID;
    GLuint AnimatedindexBufferID;
    GLuint AnimatedcolorBufferID;
    GLuint AnimatednormalBufferID;
    GLuint AnimateduvBufferID;
    GLuint jointMatricesID;
    GLuint boneWeightsID;
    GLuint boneIndicesID;

    std::vector<Vertex> copiedVertices;

    struct AnimationData animationData;
    int currentAnimationTrack = -1; // Index of the current animation (-1 if no animation is playing)
    float animationTime = 0.0f;    // Time tracker for the current animation
    bool isAnimationPlaying = false; // Indicates if an animation is currently playing
    bool hasAnimation = false;

    Entity(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, GLuint staticShader, GLuint animatedShader) {
        this->position = position;
        this->scale = scale;
        this->rotation = rotation;
        this->staticID = staticShader;
        this->animatedID = animatedShader;
        this->hasAnimation = false;
    }

    void loadModelData(const std::vector<GLfloat>& vertices,
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

        transformNormals(normal_buffer_data, numVertices, computeModelMatrix(position, rotation, scale));
        initializeBuffers();
        mvpMatrixID = glGetUniformLocation(staticID, "MVP");
        modelMatrixID = glGetUniformLocation(staticID, "modelMatrix");

        AnimatedmvpMatrixID = glGetUniformLocation(animatedID, "MVP");
        AnimatedmodelMatrixID = glGetUniformLocation(animatedID, "modelMatrix");
        jointMatricesID = glGetUniformLocation(animatedID, "jointMatrix");

    }

    void initializeBuffers(){
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

        // Create a vertex buffer object to store the vertex data
        glGenBuffers(1, &AnimatedvertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, AnimatedvertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, numVertices*3*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW);

        // Create a vertex buffer object to store the color data
        glGenBuffers(1, &AnimatedcolorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, AnimatedcolorBufferID);
        glBufferData(GL_ARRAY_BUFFER, numVertices*3*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);

        // Create a vertex buffer object to store the UV data
        glGenBuffers(1, &AnimateduvBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, AnimateduvBufferID);
        glBufferData(GL_ARRAY_BUFFER, numVertices*2*sizeof(GLfloat), uv_buffer_data, GL_STATIC_DRAW);

        // Create an index buffer object to store the index data that defines triangle faces
        glGenBuffers(1, &AnimatedindexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AnimatedindexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices*sizeof(GLfloat), index_buffer_data, GL_STATIC_DRAW);

        glGenBuffers(1, &AnimatednormalBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, AnimatednormalBufferID);
        glBufferData(GL_ARRAY_BUFFER, numVertices*3*sizeof(GLfloat), normal_buffer_data, GL_STATIC_DRAW);

        glGenBuffers(1, &boneWeightsID);
        glBindBuffer(GL_ARRAY_BUFFER, boneWeightsID);

        // Create a vertex buffer object to store the UV data
        glGenBuffers(1, &boneIndicesID);
        glBindBuffer(GL_ARRAY_BUFFER, boneIndicesID);
    }

    void setTexture(const std::string& texturePath){
        textureID = LoadTextureTileBox(texturePath.c_str());
    }

    void loadAnimationData(const AnimationData animationData) {
        this->animationData = animationData;
        for (int i = 0; i < animationData.vertices.size(); ++i) {
            copiedVertices.push_back(animationData.vertices[i]);
        }
        this->hasAnimation = true;


    }

    void playAnimation(int track){
        currentAnimationTrack = track;
        animationTime = glfwGetTime();
        isAnimationPlaying = true;
    }

    glm::mat4 getBoneNodeTransform(const BoneNode& boneNode) {
        glm::mat4 transform(1.0f);

        // Combine transformations if they exist in the BoneNode
        if (boneNode.localTransform != glm::mat4(1.0f)){
            transform = boneNode.localTransform;
        } else {
            // Apply translation
            glm::vec3 translation = glm::vec3(boneNode.offsetMatrix[3]); // Extract translation from offsetMatrix
            if (translation != glm::vec3(0.0f)) {
                transform = glm::translate(transform, translation);
            }

            // Apply rotation (if stored as part of localTransform or in quaternion form, use it)
            glm::quat rotation = glm::quat_cast(boneNode.offsetMatrix); // Extract rotation from offsetMatrix
            if (glm::length(rotation) != 0.0f) {
                transform *= glm::mat4_cast(rotation);
            }

            // Apply scaling
            glm::vec3 scale = glm::vec3(
                    glm::length(glm::vec3(boneNode.offsetMatrix[0])),
                    glm::length(glm::vec3(boneNode.offsetMatrix[1])),
                    glm::length(glm::vec3(boneNode.offsetMatrix[2]))
            );
            if (scale != glm::vec3(1.0f)) {
                transform = glm::scale(transform, scale);
            }
        }

        return transform;
    }

    void computeLocalNodeTransform(const AnimationData& animationData,
                                   int nodeIndex,
                                   std::vector<glm::mat4>& localTransforms)
    {
        const BoneNode& bone = animationData.bones[nodeIndex];
        localTransforms[nodeIndex] = getBoneNodeTransform(bone);
    }

    void computeGlobalBoneTransform(
            const AnimationData& animationData,
            const std::vector<glm::mat4>& localTransforms,
            int boneIndex,
            const glm::mat4& parentTransform,
            std::vector<glm::mat4>& globalTransforms
    ) {
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



    void update(float deltaTime) {
        if (currentAnimationTrack >= 0 && isAnimationPlaying) {
            // Advance animation time
            animationTime += deltaTime;

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
                const glm::mat4& offsetMatrix = glm::transpose(bone.offsetMatrix);
                //std::cout << "Bone: " << bone.name<< ", index: " << bone.index << "\n";
                //printMatrix(offsetMatrix);
                //printMatrix(globalTransform);

                animationData.bones[i].finalTransformation = globalTransform * offsetMatrix;
                //animationData.bones[i].finalTransformation = globalTransform * offsetMatrix ;

                /*
                if (i == 44){
                    //printMatrix(globalTransform);
                    //printMatrix(offsetMatrix);
                    int x = 0;
                    //animationData.bones[i].finalTransformation = globalTransform * offsetMatrix;
                } else {
                    animationData.bones[i].finalTransformation = glm::mat4(1.0);
                }
                 */




            }
            int x = 0;
            /*
            for (size_t i = 0; i < copiedVertices.size(); ++i) {
                Vertex& vertex = copiedVertices[i];
                glm::mat4 skinMat = glm::mat4(0.0f);
                float boneSum = 0.0f;

                for (int j = 0; j < 4; ++j) {
                    int boneID = vertex.boneIDs[j];
                    float boneWeight = vertex.boneWeights[j];

                    if (boneID < 0 || boneWeight <= 0.0f) {
                        continue; // Skip invalid bones
                    }

                    //const glm::mat4& finalTransform = glm::transpose(animationData.bones[boneID].finalTransformation);
                    const glm::mat4& finalTransform = animationData.bones[boneID].finalTransformation;
                    skinMat += finalTransform * boneWeight;
                    boneSum += boneWeight;

                    // Debugging output
                    //std::cout << "Bone: " << animationData.bones[boneID].name << ", Weight: " << boneWeight << "\n";
                    //printMatrix(finalTransform);
                }

                // Normalize the skinning matrix
                if (boneSum > 0.0f) {
                    skinMat /= boneSum;
                }

                // Apply the skinning transformation
                glm::vec4 transformed = skinMat * glm::vec4(vertex.position, 1.0f);
                glm::vec3 transformedPosition = glm::vec3(transformed);

                // Debug final position
                //std::cout << "Transformed position: " << transformedPosition.x << ", "
                //          << transformedPosition.y << ", " << transformedPosition.z << "\n";

                // Update the vertex data
                animationData.vertices[i].position = transformedPosition;
            }
        */



        }
    }

    int findKeyframeIndex(const std::vector<float>& times, float animationTime)
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

    void updateAnimation(float time, std::vector<glm::mat4> &nodeTransforms) {
        if (currentAnimationTrack < 0 || currentAnimationTrack >= animationData.animations.size()) {
            return; // Invalid track, do nothing
        }
        const AnimationTrack &animation = animationData.animations[currentAnimationTrack];
        time = 0.0;
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
                //glm::vec3 interpolatedPosition = keyframe0.position;
                //glm::quat interpolatedRotation = keyframe0.rotation;
                //glm::vec3 interpolatedScale = keyframe0.scale;
                // Construct the local transform matrix
                localTransformPos = glm::translate(glm::mat4(1.0f), interpolatedPosition);
                localTransformRot = glm::mat4_cast(interpolatedRotation);
                localTransformSca = glm::scale(glm::mat4(1.0f), interpolatedScale);

                localTransform = localTransformPos * localTransformRot * localTransformSca;
                nodeTransforms[boneNode.index] = localTransform;
            }



        }
    }



    void render(glm::mat4 cameraMatrix) {
        if (isAnimationPlaying){
            glUseProgram(animatedID);


            modelMatrix = computeModelMatrix(position, rotation, scale);




            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, AnimatedcolorBufferID);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

            // Enable UV buffer and texture sampler
            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, AnimateduvBufferID);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

            glEnableVertexAttribArray(3);
            glBindBuffer(GL_ARRAY_BUFFER, AnimatednormalBufferID);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

            std::vector<glm::vec4> boneWeightsData;
            std::vector<glm::vec4> boneIndicesData;
            GLfloat * new_vertex_buffer_data = new GLfloat[numVertices*3];
            int i = 0;
            for (const auto& vertex : animationData.vertices) {
                new_vertex_buffer_data[i] = vertex.position.x;
                new_vertex_buffer_data[i+1] = vertex.position.y;
                new_vertex_buffer_data[i+2] = vertex.position.z;
                i+=3;
                glm::vec4 floatIndices = glm::vec4((GLfloat) vertex.boneIDs.x, (GLfloat) vertex.boneIDs.y, (GLfloat) vertex.boneIDs.z, (GLfloat) vertex.boneIDs.w);
                boneWeightsData.push_back(vertex.boneWeights); // Store bone weights
                boneIndicesData.push_back(floatIndices);    // Store bone indices
            }

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, AnimatedvertexBufferID);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            //glBufferData(GL_ARRAY_BUFFER, numVertices*3*sizeof(GLfloat), new_vertex_buffer_data, GL_STATIC_DRAW);

            // Pass bone weights to the shader
            glEnableVertexAttribArray(4); // Enable the attribute for bone weights
            glBindBuffer(GL_ARRAY_BUFFER, boneWeightsID); // Assuming the buffer is already created and bound
            glBufferData(GL_ARRAY_BUFFER, boneWeightsData.size() * sizeof(glm::vec4), boneWeightsData.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0); // Location 3 for bone weights


            // Pass bone indices to the shader
            glEnableVertexAttribArray(5); // Enable the attribute for bone indices
            glBindBuffer(GL_ARRAY_BUFFER, boneIndicesID); // Assuming the buffer is already created and bound
            glBufferData(GL_ARRAY_BUFFER, boneIndicesData.size() * sizeof(glm::vec4), boneIndicesData.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0); // Location 4 for bone indices





            std::vector<float> jointMatrixData;
            for (const auto& bone : animationData.bones) {
                const float* matPtr = glm::value_ptr(bone.finalTransformation);
                jointMatrixData.insert(jointMatrixData.end(), matPtr, matPtr + 16);
            }

            glUniformMatrix4fv(jointMatricesID, animationData.bones.size(), GL_FALSE, jointMatrixData.data());


            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AnimatedindexBufferID);


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

    void lightRender(GLuint lightShader, Light light) {
        glUseProgram(lightShader);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

        glm::mat4 modelMatrix = computeModelMatrix(position, rotation, scale);
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "modelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "shadowMatrices"), 6, GL_FALSE, &light.VPmatrices[0][0][0]);

        glUniform3fv(glGetUniformLocation(lightShader, "lightPos"), 1, &light.position[0]);
        glUniform1f(glGetUniformLocation(lightShader, "farPlane"), light.lightRange);

        // Draw the box
        glDrawElements(
                GL_TRIANGLES,      // mode
                numIndices,    	// number of indices
                GL_UNSIGNED_INT,   // type
                (void*)0           // element array buffer offset
        );

        glDisableVertexAttribArray(0);
    }
    void cleanup() {
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteBuffers(1, &colorBufferID);
        glDeleteBuffers(1, &indexBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);
        glDeleteBuffers(1, &uvBufferID);
        glDeleteTextures(1, &textureID);
    }

};

class Plane : public Geometry {
public:
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;

    static const GLuint numVertices =4;
    static const GLuint numIndices = 6;
    GLfloat vertex_buffer_data[numVertices*3] = {
            -0.5f, 0.0f, 0.5f,
            0.5f, 0.0f, 0.5f,
            0.5f, 0.0f, -0.5f,
            -0.5f, 0.0f, -0.5f,
    };

    GLfloat normal_buffer_data[numVertices*3];

    GLfloat color_buffer_data[numVertices*3] = {
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
    };


    GLuint index_buffer_data[numIndices] = {
            0, 1, 2,
            0, 2, 3,
    };


    GLfloat uv_buffer_data[(numVertices)*2] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
    };

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
    GLuint textureSamplerID;
    GLuint modelMatrixID;
    //static GLuint programID;

    Plane(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, GLuint programID)
            : Geometry(position, scale, rotation, programID) {
        this->position = position;
        this->scale = scale;
        this->rotation = rotation;
        this->programID = programID;
    }

    void initialize(const char *texture_file_path) {

        this->position = position;
        this->scale = scale;
        this->rotation = rotation;
        computeNormals(vertex_buffer_data, index_buffer_data, numVertices, numIndices, normal_buffer_data);
        transformNormals(normal_buffer_data, numVertices, computeModelMatrix(position, rotation, scale));
        // Create a vertex array object
        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);

        // Create a vertex buffer object to store the vertex data
        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

        // Create a vertex buffer object to store the color data
        glGenBuffers(1, &colorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

        // Create a vertex buffer object to store the UV data
        glGenBuffers(1, &uvBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

        // Create an index buffer object to store the index data that defines triangle faces
        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

        glGenBuffers(1, &normalBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(normal_buffer_data), normal_buffer_data, GL_STATIC_DRAW);

        // Create and compile our GLSL program from the shaders
        /*
        if (programID == 0)
        {
            programID = LoadShadersFromFile("../shaders/geometry.vert", "../shaders/geometry.frag");
        }
        if (programID == 0)
        {
            std::cerr << "Failed to load shaders." << std::endl;
        }
        */

        // Get a handle for our "MVP" uniform
        mvpMatrixID = glGetUniformLocation(programID, "MVP");

        modelMatrixID = glGetUniformLocation(programID, "modelMatrix");

        textureID = LoadTextureTileBox(texture_file_path);

        textureSamplerID = glGetUniformLocation(programID,"textureSampler");
    }

    void render(glm::mat4 cameraMatrix) {
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

        glm::mat4 modelMatrix = computeModelMatrix(position, rotation, scale);
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
                numIndices,    	// number of indices
                GL_UNSIGNED_INT,   // type
                (void*)0           // element array buffer offset
        );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(3);
    }

    void lightRender(GLuint lightShader, Light light) {
        glUseProgram(lightShader);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

        glm::mat4 modelMatrix = computeModelMatrix(position, rotation, scale);
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "modelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "shadowMatrices"), 6, GL_FALSE, &light.VPmatrices[0][0][0]);

        glUniform3fv(glGetUniformLocation(lightShader, "lightPos"), 1, &light.position[0]);
        glUniform1f(glGetUniformLocation(lightShader, "farPlane"), light.lightRange);

        // Draw the box
        glDrawElements(
                GL_TRIANGLES,      // mode
                numIndices,    	// number of indices
                GL_UNSIGNED_INT,   // type
                (void*)0           // element array buffer offset
        );

        glDisableVertexAttribArray(0);
    }
    void cleanup() {
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteBuffers(1, &colorBufferID);
        glDeleteBuffers(1, &indexBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);
        glDeleteBuffers(1, &uvBufferID);
        glDeleteTextures(1, &textureID);
    }
};

class Box : public Geometry {
public:
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
    static const GLuint numVertices =24;
    static const GLuint numIndices = 36;
    GLfloat vertex_buffer_data[numVertices*3] = {	// Vertex definition for a canonical box
            // Front face
            -0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,

            // Back face
            0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,

            // Left face
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, -0.5f,

            // Right face
            0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, 0.5f,

            // Top face
            -0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f, -0.5f,

            // Bottom face
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f,
    };

    GLfloat normal_buffer_data[numVertices*3];

    GLfloat color_buffer_data[numVertices*3] = {
            // Front, red
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,

            // Back, yellow
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,

            // Left, green
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,

            // Right, cyan
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f,

            // Top, blue
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,

            // Bottom, magenta
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
    };

    GLuint index_buffer_data[numIndices] = {		// 12 triangle faces of a box
            0, 1, 2,
            0, 2, 3,

            4, 5, 6,
            4, 6, 7,

            8, 9, 10,
            8, 10, 11,

            12, 13, 14,
            12, 14, 15,

            16, 17, 18,
            16, 18, 19,

            20, 21, 22,
            20, 22, 23,
    };

    GLfloat uv_buffer_data[numVertices * 2] = {
            // Top face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,

            // Bottom face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,

            // Front face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,

            // Back face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,

            // Right face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,

            // Left face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
    };

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
    //static GLuint programID;

    glm::mat4 modelMatrix;

    Box(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, GLuint programID)
            : Geometry(position, scale, rotation, programID) {
        this->position = position;
        this->scale = scale;
        this->rotation = rotation;
        this->programID = programID;
    }

    void initialize(const char *texture_file_path) {

        this->position = position;
        this->scale = scale;
        this->rotation = rotation;

        computeNormals(vertex_buffer_data, index_buffer_data, numVertices, numIndices, normal_buffer_data);
        modelMatrix = computeModelMatrix(position, rotation, scale);
        transformNormals(normal_buffer_data, numVertices, modelMatrix);
        // Create a vertex array object
        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);

        // Create a vertex buffer object to store the vertex data
        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

        // Create a vertex buffer object to store the color data
        glGenBuffers(1, &colorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

        // Create a vertex buffer object to store the UV data
        glGenBuffers(1, &uvBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

        glGenBuffers(1, &normalBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(normal_buffer_data), normal_buffer_data, GL_STATIC_DRAW);

        // Create an index buffer object to store the index data that defines triangle faces
        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

        // Create and compile our GLSL program from the shaders
        /*
        if (programID == 0)
        {
            programID = LoadShadersFromFile("../shaders/geometry.vert", "../shaders/geometry.frag");
        }
        if (programID == 0)
        {
            std::cerr << "Failed to load shaders." << std::endl;
        }
        */

        // Get a handle for our "MVP" uniform
        mvpMatrixID = glGetUniformLocation(programID, "MVP");

        modelMatrixID = glGetUniformLocation(programID, "modelMatrix");

        textureID = LoadTextureTileBox(texture_file_path);

        textureSamplerID = glGetUniformLocation(programID,"textureSampler");
    }

    void render(glm::mat4 cameraMatrix) {
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



        // Set textureSampler to use texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(textureSamplerID, 0);

        // Draw the box
        glDrawElements(
                GL_TRIANGLES,      // mode
                numIndices,    	// number of indices
                GL_UNSIGNED_INT,   // type
                (void*)0           // element array buffer offset
        );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(3);
    }

    void lightRender(GLuint lightShader, Light light) {
        glUseProgram(lightShader);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

        glm::mat4 modelMatrix = computeModelMatrix(position, rotation, scale);
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "modelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "shadowMatrices"), 6, GL_FALSE, &light.VPmatrices[0][0][0]);

        glUniform3fv(glGetUniformLocation(lightShader, "lightPos"), 1, &light.position[0]);
        glUniform1f(glGetUniformLocation(lightShader, "farPlane"), light.lightRange);

        // Draw the box
        glDrawElements(
                GL_TRIANGLES,      // mode
                numIndices,    	// number of indices
                GL_UNSIGNED_INT,   // type
                (void*)0           // element array buffer offset
        );

        glDisableVertexAttribArray(0);
    }

    void cleanup() {
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteBuffers(1, &colorBufferID);
        glDeleteBuffers(1, &indexBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);
        glDeleteBuffers(1, &uvBufferID);
        glDeleteTextures(1, &textureID);
    }
};

class DeferredShader {
public:
    // Vertex buffer data for a fullscreen quad
    GLfloat vertex_buffer_data[16] = {
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f,  1.0f,  1.0f, 1.0f,
    };

    GLuint index_buffer_data[6] = {
            0, 1, 2,
            2, 1, 3,
    };

    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;

    GLuint programID;
    GLuint gPositionID;
    GLuint gNormalID;
    GLuint gAlbedoSpecID;
    GLuint lightPositionID;
    GLuint lightColorID;
    GLuint lightIntensityID;

    DeferredShader(GLuint programID) {
        this->programID = programID;
    }

    void initialize() {

        // Generate and bind the vertex array object
        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);

        // Generate and bind the vertex buffer object
        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

        // Generate and bind the index buffer object
        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

        // Get uniform locations for G-buffer textures
        gPositionID = glGetUniformLocation(programID, "gPosition");
        gNormalID = glGetUniformLocation(programID, "gNormal");
        gAlbedoSpecID = glGetUniformLocation(programID, "gAlbedoSpec");

        // Get uniform locations for light properties
        lightPositionID = glGetUniformLocation(programID, "lightPosition");
        lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
        lightColorID = glGetUniformLocation(programID, "lightColour");
    }

    void render(GLuint gBuffer, Light theLight, GLuint gColour, GLuint gPosition, GLuint gNormal) {
        // Use the shader program
        glUseProgram(programID);



        // Bind G-buffer textures to texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gColour);
        glUniform1i(gAlbedoSpecID, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glUniform1i(gPositionID, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glUniform1i(gNormalID, 2);

        // Bind the depth map (cubemap) to a texture unit
        glActiveTexture(GL_TEXTURE3); // Choose an unused texture unit
        glBindTexture(GL_TEXTURE_CUBE_MAP, theLight.shadowCubeMap); // shadowFBO should be the cubemap texture
        glUniform1i(glGetUniformLocation(programID, "depthMap"), 3); // Ensure the shader knows the correct texture unit

        // Set light properties
        glUniform3fv(lightPositionID, 1, &theLight.position[0]);
        glUniform3fv(lightColorID, 1, &theLight.colour[0]);
        glUniform1f(lightIntensityID, theLight.intensity);
        glUniform1f(glGetUniformLocation(programID, "lightRange"), theLight.lightRange);
        // Bind the vertex array and set up vertex attributes
        glBindVertexArray(vertexArrayID);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

        // Render the fullscreen quad
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }

    void cleanup() {
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteBuffers(1, &indexBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);
    }
};


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
void createGBuffer(GLuint* gBuffer, GLuint*  gColour, GLuint* gPosition, GLuint* gNormal, GLuint* rboDepth, int width, int height) {


    //initialize fbo and corresponding textures;
    glGenFramebuffers(1, gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, *gBuffer);

    glGenTextures(1, gColour);
    glBindTexture(GL_TEXTURE_2D, *gColour);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *gColour, 0);

    glGenTextures(1, gPosition);
    glBindTexture(GL_TEXTURE_2D, *gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, *gPosition, 0);

    glGenTextures(1, gNormal);
    glBindTexture(GL_TEXTURE_2D, *gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, *gNormal, 0);

    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, *rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *rboDepth);


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


bool loadModelWithAssimp(const std::string& objFilePath,
                         std::vector<GLfloat>& vertices,
                         std::vector<GLfloat>& normals,
                         std::vector<GLfloat>& uvs,
                         std::vector<GLuint>& indices) {
    // Create an Assimp Importer instance
    Assimp::Importer importer;

    // Load the model with Assimp
    const aiScene* scene = importer.ReadFile(
            objFilePath,
            aiProcess_Triangulate |          // Convert all faces to triangles
            aiProcess_JoinIdenticalVertices | // Combine duplicate vertices
            aiProcess_GenSmoothNormals |     // Generate normals if missing
            aiProcess_FlipUVs                // Flip UVs (if necessary for your renderer)
    );

    // Check if the model was loaded successfully
    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "ERROR: Assimp failed to load model: " << importer.GetErrorString() << std::endl;
        return false;
    }

    // Assimp loads the model as a scene graph; process the root node
    aiMesh* mesh = scene->mMeshes[0]; // Assuming the model contains only one mesh
    if (!mesh) {
        std::cerr << "ERROR: No mesh found in the file." << std::endl;
        return false;
    }

    // Extract vertices, normals, and UVs
    vertices.reserve(mesh->mNumVertices * 3); // Each vertex has 3 components: x, y, z
    normals.reserve(mesh->mNumVertices * 3); // Each normal has 3 components: nx, ny, nz
    uvs.reserve(mesh->mNumVertices * 2);     // Each UV has 2 components: u, v

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        // Vertex position
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);

        // Normal vector
        if (mesh->HasNormals()) {
            normals.push_back(mesh->mNormals[i].x);
            normals.push_back(mesh->mNormals[i].y);
            normals.push_back(mesh->mNormals[i].z);
        } else {
            normals.push_back(0.0f);
            normals.push_back(0.0f);
            normals.push_back(0.0f);
        }

        // Texture coordinates
        if (mesh->HasTextureCoords(0)) {
            uvs.push_back(mesh->mTextureCoords[0][i].x);
            uvs.push_back(mesh->mTextureCoords[0][i].y);
        } else {
            uvs.push_back(0.0f);
            uvs.push_back(0.0f);
        }
    }

    // Extract indices
    indices.reserve(mesh->mNumFaces * 3); // Each face is a triangle with 3 indices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        if (face.mNumIndices == 3) { // Ensure the face is a triangle
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }
    }

    // Load texture (optional: Assimp can extract texture information, but actual loading depends on your library)
    if (scene->HasMaterials()) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiString texturePath;

        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
            std::cout << "Texture Path: " << texturePath.C_Str() << std::endl;
            // You can use this path to load the texture (e.g., with SOIL, stb_image, etc.)
            // Note: Texture paths are relative to the OBJ file location
        }
    }

    return true;
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
                boneInfo.offsetMatrix = (glm::make_mat4(&mesh->mBones[i]->mOffsetMatrix.a1));
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

    GLuint gBuffer, gColour, gPosition, gNormal, rboDepth;


    // Create and configure the G-buffer
    createGBuffer(&gBuffer, &gColour, &gPosition, &gNormal, &rboDepth, screenWidth, screenHeight);


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

    Plane * groundPlane = new Plane(glm::vec3(0,0,0), glm::vec3(640, 1, 640), glm::vec3(0,0,0), geometry_programID);
    groundPlane->initialize(concrete_texture);
    geometries.push_back(groundPlane);

    Plane * wall = new Plane(glm::vec3(0,0,-320), glm::vec3(480, 1, 640), glm::vec3(90,0,0), geometry_programID);
    wall->initialize(concrete_texture);
    geometries.push_back(wall);

    Box * testBox1 = new Box(glm::vec3(128,32,0), glm::vec3(64, 64, 64), glm::vec3(0,45,0), geometry_programID);
    testBox1->initialize(concrete_texture);
    geometries.push_back(testBox1);

    Box * testBox2 = new Box(glm::vec3(0,80,-160), glm::vec3(32, 64, 48), glm::vec3(45,45,0), geometry_programID);
    testBox2->initialize(concrete_texture);
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

    Box * lightBox = new Box(lightPosition, glm::vec3(4, 4, 4), glm::vec3(0,0,0), geometry_programID);
    lightBox->initialize(concrete_texture);
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

        for (Geometry * g:geometries) {
            g->render(vp);
        }
        for (Entity * e:entities) {
            e->update(time);
            e->render(vp);
        }



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
                e->lightRender(shadowMap_shader, *l);
            }


        }


        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            //For debugging purposed
            //saveCubeMapDepthTexture(testLight->shadowFBO, testLight->shadowCubeMap, "depther.png");
            saveGBufferTextures(gBuffer, gColour, gPosition, gNormal, rboDepth,1920, 1080);
            saveDepthCubemapToImage(testLight->shadowCubeMap, "light_depth_map.png");


        }

        /**/
        //Render to Screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, screenWidth, screenHeight);


        //Implmenet Lighting Pass
        deferredShader->render(gBuffer, *testLight, gColour, gPosition, gNormal);
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
