//
// Created by rowan on 22/12/2024.
//
#include <algorithm>
#include "primitives.h"
MeshData::MeshData(const int numVertices, const int numIndices, float *vertexBufferData, float *normalBufferData,
                   float *colorBufferData, unsigned int *indexBufferData, float *uvBufferData) : numVertices(
        numVertices), numIndices(numIndices), vertex_buffer_data(vertexBufferData), normal_buffer_data(
        normalBufferData), color_buffer_data(colorBufferData), index_buffer_data(indexBufferData), uv_buffer_data(
        uvBufferData) {}

MeshData deepCopyMeshData(const MeshData& original) {
    // Allocate new memory for each array and copy the contents from the original MeshData
    GLfloat* copiedVertexBufferData = new GLfloat[original.numVertices * 3];
    std::copy(original.vertex_buffer_data, original.vertex_buffer_data + original.numVertices * 3, copiedVertexBufferData);

    GLfloat* copiedNormalBufferData = new GLfloat[original.numVertices * 3];
    std::copy(original.normal_buffer_data, original.normal_buffer_data + original.numVertices * 3, copiedNormalBufferData);

    GLfloat* copiedColorBufferData = new GLfloat[original.numVertices * 3];
    std::copy(original.color_buffer_data, original.color_buffer_data + original.numVertices * 3, copiedColorBufferData);

    GLuint* copiedIndexBufferData = new GLuint[original.numIndices];
    std::copy(original.index_buffer_data, original.index_buffer_data + original.numIndices, copiedIndexBufferData);

    GLfloat* copiedUVBufferData = new GLfloat[original.numVertices * 2];
    std::copy(original.uv_buffer_data, original.uv_buffer_data + original.numVertices * 2, copiedUVBufferData);

    // Return a new MeshData instance with the copied data
    return MeshData(
            original.numVertices,
            original.numIndices,
            copiedVertexBufferData,
            copiedNormalBufferData,
            copiedColorBufferData,
            copiedIndexBufferData,
            copiedUVBufferData
    );
}
