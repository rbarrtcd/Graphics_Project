
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#ifndef LAB4_PRIMITIVES_H
#define LAB4_PRIMITIVES_H


struct MeshData {
    MeshData(const int numVertices, const int numIndices, GLfloat *vertexBufferData, GLfloat *normalBufferData,
             GLfloat *colorBufferData, GLuint *indexBufferData, GLfloat *uvBufferData);

    const int numVertices;
    const int numIndices;
    GLfloat * vertex_buffer_data;
    GLfloat * normal_buffer_data;
    GLfloat * color_buffer_data;
    GLuint * index_buffer_data;
    GLfloat * uv_buffer_data;
};


// Static constant arrays for box geometry
static const GLfloat box_vertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

        // Back face
        0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,

        // Left face
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,

        // Right face
        0.5f, -0.5f,  0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f,  0.5f,

        // Top face
        -0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,

        // Bottom face
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f
};

static const GLfloat box_normals[] = {
        // Normal data (for simplicity, all normals are assumed to be unit vectors)
        0.0f, 0.0f,  1.0f,   0.0f, 0.0f,  1.0f,   0.0f, 0.0f,  1.0f,   0.0f, 0.0f,  1.0f,
        0.0f, 0.0f, -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f, -1.0f,
        -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,
        1.0f, 0.0f,  0.0f,   1.0f, 0.0f,  0.0f,   1.0f, 0.0f,  0.0f,   1.0f, 0.0f,  0.0f,
        0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f
};

static const GLfloat box_colors[] = {
        // Colors: Front = red, Back = yellow, etc.
        1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f
};

static const GLuint box_indices[] = {
        0, 1, 2,   0, 2, 3,   // Front
        4, 5, 6,   4, 6, 7,   // Back
        8, 9, 10,  8, 10, 11,  // Left
        12, 13, 14, 12, 14, 15, // Right
        16, 17, 18, 16, 18, 19, // Top
        20, 21, 22, 20, 22, 23  // Bottom
};

static const GLfloat box_uv[] = {
        // UVs for the cube
        0.0f, 0.0f,   1.0f, 0.0f,   1.0f, 1.0f,   0.0f, 1.0f, // Front
        0.0f, 0.0f,   1.0f, 0.0f,   1.0f, 1.0f,   0.0f, 1.0f, // Back
        0.0f, 0.0f,   1.0f, 0.0f,   1.0f, 1.0f,   0.0f, 1.0f, // Left
        0.0f, 0.0f,   1.0f, 0.0f,   1.0f, 1.0f,   0.0f, 1.0f, // Right
        0.0f, 0.0f,   1.0f, 0.0f,   1.0f, 1.0f,   0.0f, 1.0f, // Top
        0.0f, 0.0f,   1.0f, 0.0f,   1.0f, 1.0f,   0.0f, 1.0f  // Bottom
};


static const MeshData meshData_box(
        24, // numVertices
        36, // numIndices
        const_cast<GLfloat*>(box_vertices),
        const_cast<GLfloat*>(box_normals),
        const_cast<GLfloat*>(box_colors),
        const_cast<GLuint*>(box_indices),
        const_cast<GLfloat*>(box_uv)
);

static const GLfloat plane_vertices[] = {
        -0.5f, 0.0f,  0.5f,  // Bottom left
        0.5f, 0.0f,  0.5f,  // Bottom right
        0.5f, 0.0f, -0.5f,  // Top right
        -0.5f, 0.0f, -0.5f   // Top left
};

static const GLfloat plane_normals[] = {
        0.0f, 1.0f,  0.0f,
        0.0f, 1.0f,  0.0f,
        0.0f, 1.0f,  0.0f,
        0.0f, 1.0f,  0.0f
};

static const GLfloat plane_colors[] = {
        1.0f, 1.0f, 1.0f,  // White color for each vertex
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f
};

static const GLuint plane_indices[] = {
        0, 1, 2,   // First triangle
        0, 2, 3    // Second triangle
};

static const GLfloat plane_uv[] = {
        0.0f, 0.0f,   // Bottom-left UV
        1.0f, 0.0f,   // Bottom-right UV
        1.0f, 1.0f,   // Top-right UV
        0.0f, 1.0f    // Top-left UV
};

static MeshData meshData_plane(
        4,  // Number of vertices
        6,  // Number of indices (2 triangles)
        const_cast<GLfloat*>(plane_vertices),
        const_cast<GLfloat*>(plane_normals),
        const_cast<GLfloat*>(plane_colors),
        const_cast<GLuint*>(plane_indices),
        const_cast<GLfloat*>(plane_uv)
);



#endif //LAB4_PRIMITIVES_H