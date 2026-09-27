#pragma once
#include "rendering/vertexDataShape.h"
class Vertices : public Asset
{
public:

     Vertices() = default;


     // ============================================================
     // Set Preset
     // ============================================================

     void OnLoadedBySerialization() {
          SetPreset(currentPreset);
     }

     void SetPreset(int preset);


     // ============================================================
     // Create Mesh
     // ============================================================

     Mesh CreateMesh() const
     {
          switch (currentPreset)
          {
          case 0:

               return CreateMeshFromData(
                    cubeVertices,
                    sizeof(cubeVertices),
                    cubeIndices,
                    sizeof(cubeIndices),
                    8,
                    36
               );


          case 1:

               return CreateMeshFromData(
                    triangleVertices,
                    sizeof(triangleVertices),
                    triangleIndices,
                    sizeof(triangleIndices),
                    3,
                    3
               );


          case 2:

               return CreateMeshFromData(
                    planeVertices,
                    sizeof(planeVertices),
                    planeIndices,
                    sizeof(planeIndices),
                    4,
                    6
               );
          }


          // 默认 Cube
          return CreateMeshFromData(
               cubeVertices,
               sizeof(cubeVertices),
               cubeIndices,
               sizeof(cubeIndices),
               8,
               36
          );
     }


     int RenderSystemIndex = -1;


private:

     // ============================================================
     // Create Mesh From Data
     // ============================================================

     Mesh CreateMeshFromData(
          const float* vertices,
          size_t vertexDataSize,
          const unsigned int* indices,
          size_t indexDataSize,
          unsigned int vertexCount,
          unsigned int indexCount
     ) const
     {
          Mesh mesh;


          // ========================================================
          // Vertex Data
          // ========================================================

          mesh.vertexData.resize(
               vertexDataSize
          );

          std::memcpy(
               mesh.vertexData.data(),
               vertices,
               vertexDataSize
          );


          // ========================================================
          // Index Data
          // ========================================================

          mesh.indexData.resize(
               indexDataSize
          );

          std::memcpy(
               mesh.indexData.data(),
               indices,
               indexDataSize
          );


          // ========================================================
          // Vertex Layout
          // ========================================================

          mesh.vertexLayout.stride[0] =
               3 * sizeof(float);

          mesh.vertexLayout.attributes.push_back({
               VertexSemantic::Position,
               VertexFormat::Float3,
               0,
               0
               });


          mesh.vertexCount = vertexCount;
          mesh.indexCount = indexCount;


          return mesh;
     }


private:

     // ============================================================
     // Current Preset
     // ============================================================

     int currentPreset = 0;


     // ============================================================
     // Cube Vertex
     // ============================================================

     inline static const float cubeVertices[24] = {

          // Front
          -0.5f, -0.5f,  0.5f,
           0.5f, -0.5f,  0.5f,
           0.5f,  0.5f,  0.5f,
          -0.5f,  0.5f,  0.5f,

          // Back
          -0.5f, -0.5f, -0.5f,
           0.5f, -0.5f, -0.5f,
           0.5f,  0.5f, -0.5f,
          -0.5f,  0.5f, -0.5f
     };


     inline static const unsigned int cubeIndices[36] = {

          // Front
          0, 1, 2,
          2, 3, 0,

          // Back
          4, 6, 5,
          6, 4, 7,

          // Left
          4, 0, 3,
          3, 7, 4,

          // Right
          1, 5, 6,
          6, 2, 1,

          // Top
          3, 2, 6,
          6, 7, 3,

          // Bottom
          4, 5, 1,
          1, 0, 4
     };


     // ============================================================
     // Triangle Vertex
     // ============================================================

     inline static const float triangleVertices[9] = {

          // Bottom Left
          -0.5f, -0.5f, 0.0f,

          // Bottom Right
           0.5f, -0.5f, 0.0f,

           // Top
            0.0f,  0.5f, 0.0f
     };


     inline static const unsigned int triangleIndices[3] = {

          0, 1, 2
     };


     // ============================================================
     // Plane Vertex
     // ============================================================

     inline static const float planeVertices[12] = {

          // Bottom Left
          -0.5f, 0.0f, -0.5f,

          // Bottom Right
           0.5f, 0.0f, -0.5f,

           // Top Right
            0.5f, 0.0f,  0.5f,

            // Top Left
            -0.5f, 0.0f,  0.5f
     };


     inline static const unsigned int planeIndices[6] = {

          0, 1, 2,
          2, 3, 0
     };


     REFLECT_FRIEND(Vertices);
};


REFLECT_BASE(
     Vertices,
     Asset,
     FIELD(Vertices, currentPreset)
)


REGISTER_CLASS(Vertices)