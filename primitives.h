
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


static const GLfloat road_vertices[] = {
        // Left sidewalk
        -0.5f,  0.0f,  0.5f,  // 0: Bottom left of left sidewalk
        -0.3f,  0.0f,  0.5f,  // 1: Bottom right of left sidewalk
        -0.3f,  0.0f, -0.5f,  // 2: Top right of left sidewalk
        -0.5f,  0.0f, -0.5f,  // 3: Top left of left sidewalk

        // Left sidewalk wall (to road level)
        -0.3f,  0.0f,  0.5f,  // 4: Top left of wall
        -0.3f, -2.0f,  0.5f,  // 5: Bottom left of wall
        -0.3f, -2.0f, -0.5f,  // 6: Bottom right of wall
        -0.3f,  0.0f, -0.5f,  // 7: Top right of wall

        // Road
        -0.3f, -2.0f,  0.5f,  // 8: Bottom left of road
        0.3f, -2.0f,  0.5f,  // 9: Bottom right of road
        0.3f, -2.0f, -0.5f,  // 10: Top right of road
        -0.3f, -2.0f, -0.5f,  // 11: Top left of road

        // Left sidewalk wall (to road level)
        0.3f,  0.0f,  0.5f,  // 12: Top left of wall
        0.3f, -2.0f,  0.5f,  // 13: Bottom left of wall
        0.3f, -2.0f, -0.5f,  // 14: Bottom right of wall
        0.3f,  0.0f, -0.5f,  // 15: Top right of wall

        // Right sidewalk
        0.3f,  0.0f,  0.5f,  // 16: Bottom left of right sidewalk
        0.5f,  0.0f,  0.5f,  // 17: Bottom right of right sidewalk
        0.5f,  0.0f, -0.5f,  // 18: Top right of right sidewalk
        0.3f,  0.0f, -0.5f   // 19: Top left of right sidewalk
};

static const GLfloat road_uv[] = {
        // Left sidewalk
        0.0, 0.0,  // 0: Bottom left of left sidewalk
        0.0, 0.5,  // 1: Bottom right of left sidewalk
        5.0, 0.5,  // 2: Top right of left sidewalk (5 tiles in depth)
        5.0, 0.0,  // 3: Top left of left sidewalk

        // Left sidewalk wall (to road level)
        0.0, 0.0,  // 0: Bottom left of left sidewalk
        0.0, 0.05,  // 1: Bottom right of left sidewalk
        5.0, 0.05,  // 2: Top right of left sidewalk (5 tiles in depth)
        5.0, 0.0,  // 3: Top left of left sidewalk

        // Road
        0.0, 1.0,  // 8: Bottom left of road
        1.0, 1.0,  // 9: Bottom right of road
        1.0, 0.5,  // 10: Top right of road
        0.0, 0.5,  // 11: Top left of road

        // Left sidewalk wall (to road level)
        0.0, 0.0,  // 0: Bottom left of left sidewalk
        0.0, 0.05,  // 1: Bottom right of left sidewalk
        5.0, 0.05,  // 2: Top right of left sidewalk (5 tiles in depth)
        5.0, 0.0,  // 3: Top left of left sidewalk

        // Right sidewalk
        5.0, 0.5,  // 2: Top right of left sidewalk (5 tiles in depth)
        5.0, 0.0,  // 3: Top left of left sidewalk
        0.0, 0.0,  // 0: Bottom left of left sidewalk
        0.0, 0.5,  // 1: Bottom right of left sidewalk
};

static const GLuint road_indices[] = {
        // Left sidewalk
        0, 1, 2,  0, 2, 3,  // Two triangles

        // Left sidewalk wall
        4, 5, 6,  4, 6, 7,  // Two triangles

        // Road
        8, 9, 10,  8, 10, 11,  // Two triangles

        // Right sidewalk wall
        14, 13, 12,  15, 14, 12,  // Two triangles

        // Right sidewalk
        16, 17, 18,  16, 18, 19   // Two triangles
};

static const GLfloat road_colours[] = {
        // Left sidewalk
        1.0f, 1.0f, 1.0f,  // Color for vertex 0
        1.0f, 1.0f, 1.0f,  // Color for vertex 1
        1.0f, 1.0f, 1.0f,  // Color for vertex 2
        1.0f, 1.0f, 1.0f,  // Color for vertex 3

        // Left sidewalk wall (to road level)
        1.0f, 1.0f, 1.0f,  // Color for vertex 4
        1.0f, 1.0f, 1.0f,  // Color for vertex 5
        1.0f, 1.0f, 1.0f,  // Color for vertex 6
        1.0f, 1.0f, 1.0f,  // Color for vertex 7

        // Road
        1.0f, 1.0f, 1.0f,  // Color for vertex 8
        1.0f, 1.0f, 1.0f,  // Color for vertex 9
        1.0f, 1.0f, 1.0f,  // Color for vertex 10
        1.0f, 1.0f, 1.0f,  // Color for vertex 11

        // Right sidewalk wall (to road level)
        1.0f, 1.0f, 1.0f,  // Color for vertex 12
        1.0f, 1.0f, 1.0f,  // Color for vertex 13
        1.0f, 1.0f, 1.0f,  // Color for vertex 14
        1.0f, 1.0f, 1.0f,  // Color for vertex 15

        // Right sidewalk
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f   // Color for vertex 19
};

static const GLfloat road_normals[] = {
        // Left sidewalk
        1.0f, 1.0f, 1.0f,  // Color for vertex 0
        1.0f, 1.0f, 1.0f,  // Color for vertex 1
        1.0f, 1.0f, 1.0f,  // Color for vertex 2
        1.0f, 1.0f, 1.0f,  // Color for vertex 3

        // Left sidewalk wall (to road level)
        1.0f, 1.0f, 1.0f,  // Color for vertex 4
        1.0f, 1.0f, 1.0f,  // Color for vertex 5
        1.0f, 1.0f, 1.0f,  // Color for vertex 6
        1.0f, 1.0f, 1.0f,  // Color for vertex 7

        // Road
        1.0f, 1.0f, 1.0f,  // Color for vertex 8
        1.0f, 1.0f, 1.0f,  // Color for vertex 9
        1.0f, 1.0f, 1.0f,  // Color for vertex 10
        1.0f, 1.0f, 1.0f,  // Color for vertex 11

        // Right sidewalk wall (to road level)
        1.0f, 1.0f, 1.0f,  // Color for vertex 12
        1.0f, 1.0f, 1.0f,  // Color for vertex 13
        1.0f, 1.0f, 1.0f,  // Color for vertex 14
        1.0f, 1.0f, 1.0f,  // Color for vertex 15

        // Right sidewalk
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f   // Color for vertex 19
};

static MeshData meshData_road(
        20,  // Number of vertices
        30,  // Number of indices (2 triangles)
        const_cast<GLfloat*>(road_vertices),
        const_cast<GLfloat*>(road_normals),
        const_cast<GLfloat*>(road_colours),
        const_cast<GLuint*>(road_indices),
        const_cast<GLfloat*>(road_uv)
);


static const GLfloat tway_vertices[] = {
        // Left sidewalk
        -0.5f,  0.0f,  0.5f,  // 0: Bottom left of left sidewalk
        -0.3f,  0.0f,  0.5f,  // 1: Bottom right of left sidewalk
        -0.3f,  0.0f,  0.3f,  // 2: Top right of left sidewalk
        -0.5f,  0.0f,  0.3f,  // 3: Top left of left sidewalk

        // Left sidewalk wall (to road level)
        -0.3f,  0.0f,  0.5f,  // 4: Top left of wall
        -0.3f, -2.0f,  0.5f,  // 5: Bottom left of wall
        -0.3f, -2.0f,  0.3f,  // 6: Bottom right of wall
        -0.3f,  0.0f,  0.3f,  // 7: Top right of wall

        // Left sidewalk wall 2
        -0.3f,  0.0f,  0.3f,  // 7: Top right of wall
        -0.3f, -2.0f,  0.3f,  // 6: Bottom right of wall
        -0.5f, -2.0f,  0.3f,  // 5: Bottom left of wall
        -0.5f,  0.0f,  0.3f,  // 4: Top left of wall

        // Road 1
        -0.3f, -2.0f,  0.5f,  // 8: Bottom left of road
        0.3f, -2.0f,  0.5f,  // 9: Bottom right of road
        0.3f, -2.0f, 0.3f,  // 10: Top right of road
        -0.3f, -2.0f, 0.3f,  // 11: Top left of road

        //road 2
        -0.3f, -2.0f,  -0.5f,  // 8: Bottom left of road
        0.3f, -2.0f,  -0.5f,  // 9: Bottom right of road
        0.3f, -2.0f, -0.3f,  // 10: Top right of road
        -0.3f, -2.0f, -0.3f,  // 11: Top left of road

        //road 3
        -0.3f, -2.0f, -0.3f,  // 11: Top left of road
        -0.5f, -2.0f, -0.3f,  // 10: Top right of road
        -0.5f, -2.0f,  0.3f,  // 8: Bottom left of road
        -0.3f, -2.0f,  0.3f,  // 9: Bottom right of road

        //road square
        0.3f, -2.0f,  0.3f,  // 8: Bottom left of road
        0.3f, -2.0f, -0.3f,  // 10: Top right of road
        -0.3f, -2.0f, -0.3f,  // 11: Top left of road
        -0.3f, -2.0f,  0.3f,  // 9: Bottom right of road

        // Left sidewalk wall (to road level)
        0.3f, -2.0f, -0.5f,  // 14: Bottom right of wall
        0.3f, -2.0f,  0.5f,  // 13: Bottom left of wall
        0.3f,  0.0f,  0.5f,  // 12: Top left of wall
        0.3f,  0.0f, -0.5f,  // 15: Top right of wall

        // Right sidewalk
        0.3f,  0.0f, -0.5f,   // 19: Top left of right sidewalk
        0.3f,  0.0f,  0.5f,  // 16: Bottom left of right sidewalk
        0.5f,  0.0f,  0.5f,  // 17: Bottom right of right sidewalk
        0.5f,  0.0f, -0.5f,  // 18: Top right of right sidewalk

        // side square 2
        -0.5f,  0.0f,  -0.3f,  // 0: Bottom left of left sidewalk
        -0.3f,  0.0f,  -0.3f,  // 1: Bottom right of left sidewalk
        -0.3f,  0.0f,  -0.5f,  // 2: Top right of left sidewalk
        -0.5f,  0.0f,  -0.5f,  // 3: Top left of left sidewalk

        // side square 2 walls 1
        -0.5f,  0.0f,  -0.3f,  // 3: Top left of left sidewalk
        -0.5f,  -2.0f,  -0.3f,  // 1: Bottom right of left sidewalk
        -0.3f,  -2.0f,  -0.3f,  // 2: Top right of left sidewalk
        -0.3f,  0.0f,  -0.3f,  // 0: Bottom left of left sidewalk

        // side square 2 walls 2
        -0.3f,  0.0f,  -0.3f,  // 0: Bottom left of left sidewalk
        -0.3f,  -2.0f,  -0.3f,  // 1: Bottom right of left sidewalk
        -0.3f,  -2.0f,  -0.5f,  // 2: Top right of left sidewalk
        -0.3f,  0.0f,  -0.5f,  // 3: Top left of left sidewalk
};

static const GLfloat tway_uv[] = {
        // Left sidewalk
        0.25, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 1.0,  // 1: Bottom right of left sidewalk
        0.5, 1.0,  // 2: Top right of left sidewalk (5 tiles in depth)
        0.5, 0.0,  // 3: Top left of left sidewalk

        // Left sidewalk wall (to road level)
        0.25, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 0.05,  // 1: Bottom right of left sidewalk
        0.5, 0.05,  // 2: Top right of left sidewalk (5 tiles in depth)
        0.5, 0.0,  // 3: Top left of left sidewalk

        0.25, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 0.05,  // 1: Bottom right of left sidewalk
        0.5, 0.05,  // 2: Top right of left sidewalk (5 tiles in depth)
        0.5, 0.0,  // 3: Top left of left sidewalk

        // Road 1
        0.75, 1.0,  // 11: Top left of road
        0.75, -0.0,  // 8: Bottom left of road
        1.0, -0.0,  // 9: Bottom right of road
        1.0, 1.0,  // 10: Top right of road

        // road 2
        0.75, 1.0,  // 11: Top left of road
        0.75, -0.0,  // 8: Bottom left of road
        1.0, -0.0,  // 9: Bottom right of road
        1.0, 1.0,  // 10: Top right of road
        // road 3
        1.0, 1.0,  // 10: Top right of road
        0.75, 1.0,  // 11: Top left of road
        0.75, -0.0,  // 8: Bottom left of road
        1.0, -0.0,  // 9: Bottom right of road

        //road square
        0.0, 0.0,  // 8: Bottom left of road
        0.25, 0.0,  // 9: Bottom right of road
        0.25, 1.0,  // 10: Top right of road
        0.0, 1.0,  // 11: Top left of road

        // long side walk wall
        0.5, 0.0,  // 3: Top left of left sidewalk
        0.25, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 0.05,  // 1: Bottom right of left sidewalk
        0.5, 0.05,  // 2: Top right of left sidewalk (5 tiles in depth)

        // long side walk
        0.5, 5.0,  // 3: Top left of left sidewalk
        0.5, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 0.0,  // 1: Bottom right of left sidewalk
        0.25, 5.0,  // 2: Top right of left sidewalk (5 tiles in depth)


        //side square 2
        0.25, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 1.0,  // 1: Bottom right of left sidewalk
        0.5, 1.0,  // 2: Top right of left sidewalk (5 tiles in depth)
        0.5, 0.0,  // 3: Top left of left sidewalk

        // right sidewalk wall (to road level)
        0.25, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 0.05,  // 1: Bottom right of left sidewalk
        0.5, 0.05,  // 2: Top right of left sidewalk (5 tiles in depth)
        0.5, 0.0,  // 3: Top left of left sidewalk

        0.25, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 0.05,  // 1: Bottom right of left sidewalk
        0.5, 0.05,  // 2: Top right of left sidewalk (5 tiles in depth)
        0.5, 0.0,  // 3: Top left of left sidewalk
};

static const GLuint tway_indices[] = {
        // Left sidewalk
        0, 1, 2,  0, 2, 3,  // Two triangles

        // Left sidewalk wall
        4, 5, 6,  4, 6, 7,  // Two triangles

        // Road
        8, 9, 10,  8, 10, 11,  // Two triangles

        //road 1
        12, 13, 14,  12, 14, 15,  // Two triangles

        // road 2
        18, 17, 16,  19, 18, 16,   // Two triangles

        // road 3
        20, 21, 22, 20, 22, 23,

        // road square
        24, 25, 26, 24, 26, 27,

        // long wall
        28, 29, 30, 28, 30, 31,

        // long sidewalk
        32, 33, 34, 32, 34, 35,

        //square sidewalk 2
        36,37,38,36,38,39,
        //square sidewalk 2 wall 1
        40,41,42,40,42,43,
        //square sidewalk 2 wall 2
        44,45,46,44,46,47,
};

static const GLfloat tway_colours[] = {
        // Left sidewalk
        1.0f, 1.0f, 1.0f,  // Color for vertex 0
        1.0f, 1.0f, 1.0f,  // Color for vertex 1
        1.0f, 1.0f, 1.0f,  // Color for vertex 2
        1.0f, 1.0f, 1.0f,  // Color for vertex 3

        // Left sidewalk wall 1
        1.0f, 1.0f, 1.0f,  // Color for vertex 4
        1.0f, 1.0f, 1.0f,  // Color for vertex 5
        1.0f, 1.0f, 1.0f,  // Color for vertex 6
        1.0f, 1.0f, 1.0f,  // Color for vertex 7

        // Left sidewalk wall 2
        1.0f, 1.0f, 1.0f,  // Color for vertex 8
        1.0f, 1.0f, 1.0f,  // Color for vertex 9
        1.0f, 1.0f, 1.0f,  // Color for vertex 10
        1.0f, 1.0f, 1.0f,  // Color for vertex 11

        // road 1
        1.0f, 1.0f, 1.0f,  // Color for vertex 12
        1.0f, 1.0f, 1.0f,  // Color for vertex 13
        1.0f, 1.0f, 1.0f,  // Color for vertex 14
        1.0f, 1.0f, 1.0f,  // Color for vertex 15

        // road 2
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        // road 3
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        //road square
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        // long wall
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        //long sidewalk
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        //side square 2
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19
};

static const GLfloat tway_normals[] = {
        // Left sidewalk
        1.0f, 1.0f, 1.0f,  // Color for vertex 0
        1.0f, 1.0f, 1.0f,  // Color for vertex 1
        1.0f, 1.0f, 1.0f,  // Color for vertex 2
        1.0f, 1.0f, 1.0f,  // Color for vertex 3

        // Left sidewalk wall (to road level)
        1.0f, 1.0f, 1.0f,  // Color for vertex 4
        1.0f, 1.0f, 1.0f,  // Color for vertex 5
        1.0f, 1.0f, 1.0f,  // Color for vertex 6
        1.0f, 1.0f, 1.0f,  // Color for vertex 7

        // Road
        1.0f, 1.0f, 1.0f,  // Color for vertex 8
        1.0f, 1.0f, 1.0f,  // Color for vertex 9
        1.0f, 1.0f, 1.0f,  // Color for vertex 10
        1.0f, 1.0f, 1.0f,  // Color for vertex 11

        // Right sidewalk wall (to road level)
        1.0f, 1.0f, 1.0f,  // Color for vertex 12
        1.0f, 1.0f, 1.0f,  // Color for vertex 13
        1.0f, 1.0f, 1.0f,  // Color for vertex 14
        1.0f, 1.0f, 1.0f,  // Color for vertex 15

        // Right sidewalk
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19
};

static MeshData meshData_tway(
        48,  // Number of vertices
        72,  // Number of indices (2 triangles)
        const_cast<GLfloat*>(tway_vertices),
        const_cast<GLfloat*>(tway_normals),
        const_cast<GLfloat*>(tway_colours),
        const_cast<GLuint*>(tway_indices),
        const_cast<GLfloat*>(tway_uv)
);



static const GLfloat corner_vertices[] = {
        // close right sidewalk
        -0.5f,  0.0f,  -0.3f,  // 0: Bottom left of left sidewalk
        -0.3f,  0.0f,  -0.3f,  // 1: Bottom right of left sidewalk
        -0.3f,  0.0f,  -0.5f,  // 2: Top right of left sidewalk
        -0.5f,  0.0f,  -0.5f,  // 3: Top left of left sidewalk

        // Left sidewalk wall (to road level)
        -0.3f,  0.0f,  -0.3f,  // 7: Top right of wall
        -0.3f, -2.0f,  -0.3f,  // 6: Bottom right of wall
        -0.3f, -2.0f,  -0.5f,  // 5: Bottom left of wall
        -0.3f,  0.0f,  -0.5f,  // 4: Top left of wall

        // Left sidewalk wall 2
        -0.5f,  0.0f,  -0.3f,  // 4: Top left of wall
        -0.5f, -2.0f,  -0.3f,  // 5: Bottom left of wall
        -0.3f, -2.0f,  -0.3f,  // 6: Bottom right of wall
        -0.3f,  0.0f,  -0.3f,  // 7: Top right of wall


        //road 2
        -0.3f, -2.0f, -0.3f,  // 11: Top left of road
        0.3f, -2.0f, -0.3f,  // 10: Top right of road
        0.3f, -2.0f,  -0.5f,  // 9: Bottom right of road
        -0.3f, -2.0f,  -0.5f,  // 8: Bottom left of road

        //road 3
        -0.3f, -2.0f, -0.3f,  // 11: Top left of road
        -0.5f, -2.0f, -0.3f,  // 10: Top right of road
        -0.5f, -2.0f,  0.3f,  // 8: Bottom left of road
        -0.3f, -2.0f,  0.3f,  // 9: Bottom right of road

        //road square
        0.3f, -2.0f,  0.3f,  // 8: Bottom left of road
        0.3f, -2.0f, -0.3f,  // 10: Top right of road
        -0.3f, -2.0f, -0.3f,  // 11: Top left of road
        -0.3f, -2.0f,  0.3f,  // 9: Bottom right of road

        // left sidewalk wall
        0.3f, -2.0f, -0.5f,  // 14: Bottom right of wall
        0.3f, -2.0f,  0.3f,  // 13: Bottom left of wall
        0.3f,  0.0f,  0.3f,  // 12: Top left of wall
        0.3f,  0.0f, -0.5f,  // 15: Top right of wall

        //left sidewalk long
        0.3f,  0.0f, -0.5f,   // 19: Top left of right sidewalk
        0.3f,  0.0f,  0.3f,  // 16: Bottom left of right sidewalk
        0.5f,  0.0f,  0.3f,  // 17: Bottom right of right sidewalk
        0.5f,  0.0f, -0.5f,  // 18: Top right of right sidewalk

        // side square 2
        0.5f,  0.0f,  0.3f,  // 0: Bottom left of left sidewalk
        0.3f,  0.0f,  0.3f,  // 1: Bottom right of left sidewalk
        0.3f,  0.0f,  0.5f,  // 2: Top right of left sidewalk
        0.5f,  0.0f,  0.5f,  // 3: Top left of left sidewalk

        // long sidewalk 2
        0.3f,  0.0f, 0.5f,   // 19: Top left of right sidewalk
        0.3f,  0.0f,  0.3f,  // 16: Bottom left of right sidewalk
        -0.5f,  0.0f,  0.3f,  // 17: Bottom right of right sidewalk
        -0.5f,  0.0f, 0.5f,  // 18: Top right of right sidewalk

        // long wall 2
        0.3f, 0.0f,  0.3f,  // 13: Bottom left of wall
        0.3f, -2.0f, 0.3f,  // 14: Bottom right of wall
        -0.5f,  -2.0f,  0.3f,  // 12: Top left of wall
        -0.5f,  0.0f, 0.3f,  // 15: Top right of wall

};

static const GLfloat corner_uv[] = {
        // close right sidewalk
        0.25, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 1.0,  // 1: Bottom right of left sidewalk
        0.5, 1.0,  // 2: Top right of left sidewalk (5 tiles in depth)
        0.5, 0.0,  // 3: Top left of left sidewalk

        // Left sidewalk wall (to road level)
        0.25, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 0.05,  // 1: Bottom right of left sidewalk
        0.5, 0.05,  // 2: Top right of left sidewalk (5 tiles in depth)
        0.5, 0.0,  // 3: Top left of left sidewalk

        0.25, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 0.05,  // 1: Bottom right of left sidewalk
        0.5, 0.05,  // 2: Top right of left sidewalk (5 tiles in depth)
        0.5, 0.0,  // 3: Top left of left sidewalk

        // road 2
        0.75, 1.0,  // 11: Top left of road
        0.75, -0.0,  // 8: Bottom left of road
        1.0, -0.0,  // 9: Bottom right of road
        1.0, 1.0,  // 10: Top right of road
        // road 3
        0.75, -0.0,  // 8: Bottom left of road
        1.0, -0.0,  // 9: Bottom right of road
        1.0, 1.0,  // 10: Top right of road
        0.75, 1.0,  // 11: Top left of road

        //road square
        0.0, 0.0,  // 8: Bottom left of road
        0.25, 0.0,  // 9: Bottom right of road
        0.25, 1.0,  // 10: Top right of road
        0.0, 1.0,  // 11: Top left of road

        // long side walk wall
        0.5, 0.0,  // 3: Top left of left sidewalk
        0.25, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 0.05,  // 1: Bottom right of left sidewalk
        0.5, 0.05,  // 2: Top right of left sidewalk (5 tiles in depth)

        // long side walk
        0.5, 5.0,  // 3: Top left of left sidewalk
        0.5, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 0.0,  // 1: Bottom right of left sidewalk
        0.25, 5.0,  // 2: Top right of left sidewalk (5 tiles in depth)

        //side walk square
        0.25, 1.0,  // 1: Bottom right of left sidewalk
        0.5, 1.0,  // 2: Top right of left sidewalk (5 tiles in depth)
        0.5, 0.0,  // 3: Top left of left sidewalk
        0.25, 0.0,  // 0: Bottom left of left sidewalk

        // long side walk
        0.25, 5.0,  // 2: Top right of left sidewalk (5 tiles in depth)
        0.5, 5.0,  // 3: Top left of left sidewalk
        0.5, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 0.0,  // 1: Bottom right of left sidewalk

        // long side walk wall
        0.25, 0.0,  // 0: Bottom left of left sidewalk
        0.25, 0.05,  // 1: Bottom right of left sidewalk
        0.5, 0.05,  // 2: Top right of left sidewalk (5 tiles in depth)
        0.5, 0.0,  // 3: Top left of left sidewalk


};

static const GLuint corner_indices[] = {
        // Left sidewalk
        0, 1, 2,  0, 2, 3,  // Two triangles

        // Left sidewalk wall
        4, 5, 6,  4, 6, 7,  // Two triangles

        // Road
        8, 9, 10,  8, 10, 11,  // Two triangles

        //road 1
        12, 13, 14,  12, 14, 15,  // Two triangles

        // road 2
        16, 17, 18,  16, 18, 19,   // Two triangles

        // road 3
        20, 21, 22, 20, 22, 23,

        // road square
        24, 25, 26, 24, 26, 27,

        // long wall
        28, 29, 30, 28, 30, 31,

        // long sidewalk
        32, 33, 34, 32, 34, 35,

        //square sidewalk 2
        36,37,38,36,38,39,
        //square sidewalk 2 wall 1
        40,41,42,40,42,43,
};

static const GLfloat corner_colours[] = {
        // Left sidewalk
        1.0f, 1.0f, 1.0f,  // Color for vertex 0
        1.0f, 1.0f, 1.0f,  // Color for vertex 1
        1.0f, 1.0f, 1.0f,  // Color for vertex 2
        1.0f, 1.0f, 1.0f,  // Color for vertex 3

        // Left sidewalk wall 1
        1.0f, 1.0f, 1.0f,  // Color for vertex 4
        1.0f, 1.0f, 1.0f,  // Color for vertex 5
        1.0f, 1.0f, 1.0f,  // Color for vertex 6
        1.0f, 1.0f, 1.0f,  // Color for vertex 7

        // Left sidewalk wall 2
        1.0f, 1.0f, 1.0f,  // Color for vertex 8
        1.0f, 1.0f, 1.0f,  // Color for vertex 9
        1.0f, 1.0f, 1.0f,  // Color for vertex 10
        1.0f, 1.0f, 1.0f,  // Color for vertex 11

        // road 1
        1.0f, 1.0f, 1.0f,  // Color for vertex 12
        1.0f, 1.0f, 1.0f,  // Color for vertex 13
        1.0f, 1.0f, 1.0f,  // Color for vertex 14
        1.0f, 1.0f, 1.0f,  // Color for vertex 15

        // road 2
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        // road 3
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        //road square
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        // long wall
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        //long sidewalk
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        //side square 2
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19


};

static const GLfloat corner_normals[] = {
        // Left sidewalk
        1.0f, 1.0f, 1.0f,  // Color for vertex 0
        1.0f, 1.0f, 1.0f,  // Color for vertex 1
        1.0f, 1.0f, 1.0f,  // Color for vertex 2
        1.0f, 1.0f, 1.0f,  // Color for vertex 3

        // Left sidewalk wall (to road level)
        1.0f, 1.0f, 1.0f,  // Color for vertex 4
        1.0f, 1.0f, 1.0f,  // Color for vertex 5
        1.0f, 1.0f, 1.0f,  // Color for vertex 6
        1.0f, 1.0f, 1.0f,  // Color for vertex 7

        // Road
        1.0f, 1.0f, 1.0f,  // Color for vertex 8
        1.0f, 1.0f, 1.0f,  // Color for vertex 9
        1.0f, 1.0f, 1.0f,  // Color for vertex 10
        1.0f, 1.0f, 1.0f,  // Color for vertex 11

        // Right sidewalk wall (to road level)
        1.0f, 1.0f, 1.0f,  // Color for vertex 12
        1.0f, 1.0f, 1.0f,  // Color for vertex 13
        1.0f, 1.0f, 1.0f,  // Color for vertex 14
        1.0f, 1.0f, 1.0f,  // Color for vertex 15

        // Right sidewalk
        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

        1.0f, 1.0f, 1.0f,  // Color for vertex 16
        1.0f, 1.0f, 1.0f,  // Color for vertex 17
        1.0f, 1.0f, 1.0f,  // Color for vertex 18
        1.0f, 1.0f, 1.0f,   // Color for vertex 19

};

static MeshData meshData_corner(
        44,  // Number of vertices
        66,  // Number of indices (2 triangles)
        const_cast<GLfloat*>(corner_vertices),
        const_cast<GLfloat*>(corner_normals),
        const_cast<GLfloat*>(corner_colours),
        const_cast<GLuint*>(corner_indices),
        const_cast<GLfloat*>(corner_uv)
);



#endif //LAB4_PRIMITIVES_H