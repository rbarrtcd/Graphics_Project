//
// Created by rowan on 22/12/2024.
//
#include "primitives.h"
MeshData::MeshData(const int numVertices, const int numIndices, float *vertexBufferData, float *normalBufferData,
                   float *colorBufferData, unsigned int *indexBufferData, float *uvBufferData) : numVertices(
        numVertices), numIndices(numIndices), vertex_buffer_data(vertexBufferData), normal_buffer_data(
        normalBufferData), color_buffer_data(colorBufferData), index_buffer_data(indexBufferData), uv_buffer_data(
        uvBufferData) {}
