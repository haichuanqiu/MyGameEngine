#pragma once
#include "vertexDataShape.h"

class OpenGLVertexDataController
{
public:

     // ------------------------------------------------------------
     // CPU Mesh
     //     ↓
     // registerMesh()
     //     ↓
     // OpenGL VAO / VBO / EBO
     //     ↓
     // int
     // ------------------------------------------------------------
     int registerMesh(const Mesh& mesh)
     {
          if (mesh.vertexData.empty())
               throw std::runtime_error("Mesh has no vertex data");

          if (mesh.indexData.empty())
               throw std::runtime_error("Mesh has no index data");

          VertexDataInfo info{};


          // --------------------------------------------------------
          // 创建 VAO
          // --------------------------------------------------------

          glGenVertexArrays(1, &info.vao);
          glBindVertexArray(info.vao);


          // --------------------------------------------------------
          // 创建 VBO
          // --------------------------------------------------------

          glGenBuffers(1, &info.vbo);
          glBindBuffer(GL_ARRAY_BUFFER, info.vbo);

          glBufferData(
               GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(mesh.vertexData.size()),
               mesh.vertexData.data(),
               GL_STATIC_DRAW
          );


          // --------------------------------------------------------
          // 创建 EBO
          // --------------------------------------------------------

          glGenBuffers(1, &info.ebo);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info.ebo);

          glBufferData(
               GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(mesh.indexData.size()),
               mesh.indexData.data(),
               GL_STATIC_DRAW
          );


          // --------------------------------------------------------
          // 根据 VertexLayout 配置 VAO
          // --------------------------------------------------------

          for (const VertexAttribute& attribute :
               mesh.vertexLayout.attributes)
          {
               if (attribute.stream != 0)
               {
                    throw std::runtime_error(
                         "Multiple vertex streams are not implemented yet"
                    );
               }

               GLuint location =
                    semanticToLocation(attribute.semantic);

               GLint componentCount = 0;
               GLenum type = GL_FLOAT;
               GLboolean normalized = GL_FALSE;
               bool integer = false;

               formatToOpenGL(
                    attribute.format,
                    componentCount,
                    type,
                    normalized,
                    integer
               );


               glEnableVertexAttribArray(location);


               if (integer)
               {
                    glVertexAttribIPointer(
                         location,
                         componentCount,
                         type,
                         static_cast<GLsizei>(
                              mesh.vertexLayout.stride[0]
                              ),
                         reinterpret_cast<const void*>(
                              static_cast<uintptr_t>(
                                   attribute.offset
                                   )
                              )
                    );
               }
               else
               {
                    glVertexAttribPointer(
                         location,
                         componentCount,
                         type,
                         normalized,
                         static_cast<GLsizei>(
                              mesh.vertexLayout.stride[0]
                              ),
                         reinterpret_cast<const void*>(
                              static_cast<uintptr_t>(
                                   attribute.offset
                                   )
                              )
                    );
               }
          }


          // --------------------------------------------------------
          // VAO 配置完成
          // --------------------------------------------------------

          glBindVertexArray(0);

          // 这里不需要解绑 VBO/EBO
          // 因为后面使用 VAO 时会恢复对应状态。


          info.vertexCount = mesh.vertexCount;
          info.indexCount = mesh.indexCount;


          // --------------------------------------------------------
          // vector 下标就是这个 Mesh 的 int ID
          // --------------------------------------------------------

          int index = static_cast<int>(m_meshes.size());

          m_meshes.push_back(info);

          return index;
     }


     // ------------------------------------------------------------
     // 使用 Mesh
     // ------------------------------------------------------------
     bool drawMesh(int index)
     {
          if (!isValid(index))
               return false;

          const VertexDataInfo& info = m_meshes[index];

          glBindVertexArray(info.vao);

          glDrawElements(
               GL_TRIANGLES,
               static_cast<GLsizei>(info.indexCount),
               GL_UNSIGNED_INT,
               nullptr
          );

          return true;
     }
     bool useMesh(int index)
     {
          if (!isValid(index))
               return false;

          glBindVertexArray(m_meshes[index].vao);

          return true;
     }


     // ------------------------------------------------------------
     // 卸载 Mesh
     // ------------------------------------------------------------

     void unloadMesh(int index)
     {
          if (!isValid(index))
               return;

          VertexDataInfo& info = m_meshes[index];

          if (info.ebo != 0)
          {
               glDeleteBuffers(1, &info.ebo);
               info.ebo = 0;
          }

          if (info.vbo != 0)
          {
               glDeleteBuffers(1, &info.vbo);
               info.vbo = 0;
          }

          if (info.vao != 0)
          {
               glDeleteVertexArrays(1, &info.vao);
               info.vao = 0;
          }
     }


private:

     struct VertexDataInfo
     {
          GLuint vao = 0;
          GLuint vbo = 0;
          GLuint ebo = 0;

          uint32_t vertexCount = 0;
          uint32_t indexCount = 0;
     };


private:

     bool isValid(int index) const
     {
          return index >= 0 &&
               index < static_cast<int>(m_meshes.size()) &&
               m_meshes[index].vao != 0;
     }


     GLuint semanticToLocation(VertexSemantic semantic) const
     {
          switch (semantic)
          {
          case VertexSemantic::Position:
               return 0;

          case VertexSemantic::Normal:
               return 1;

          case VertexSemantic::Tangent:
               return 2;

          case VertexSemantic::Color:
               return 3;

          case VertexSemantic::TexCoord0:
               return 4;

          case VertexSemantic::TexCoord1:
               return 5;

          case VertexSemantic::BoneIndices:
               return 6;

          case VertexSemantic::BoneWeights:
               return 7;
          }

          throw std::runtime_error(
               "Unknown VertexSemantic"
          );
     }


     void formatToOpenGL(
          VertexFormat format,
          GLint& componentCount,
          GLenum& type,
          GLboolean& normalized,
          bool& integer) const
     {
          normalized = GL_FALSE;
          integer = false;

          switch (format)
          {
          case VertexFormat::Float:
               componentCount = 1;
               type = GL_FLOAT;
               break;

          case VertexFormat::Float2:
               componentCount = 2;
               type = GL_FLOAT;
               break;

          case VertexFormat::Float3:
               componentCount = 3;
               type = GL_FLOAT;
               break;

          case VertexFormat::Float4:
               componentCount = 4;
               type = GL_FLOAT;
               break;


          case VertexFormat::Half2:
               componentCount = 2;
               type = GL_HALF_FLOAT;
               break;

          case VertexFormat::Half4:
               componentCount = 4;
               type = GL_HALF_FLOAT;
               break;


          case VertexFormat::UInt8x4:
               componentCount = 4;
               type = GL_UNSIGNED_BYTE;
               integer = true;
               break;

          case VertexFormat::UInt16x4:
               componentCount = 4;
               type = GL_UNSIGNED_SHORT;
               integer = true;
               break;

          case VertexFormat::UInt32x4:
               componentCount = 4;
               type = GL_UNSIGNED_INT;
               integer = true;
               break;


          case VertexFormat::UNorm8x4:
               componentCount = 4;
               type = GL_UNSIGNED_BYTE;
               normalized = GL_TRUE;
               break;

          case VertexFormat::UNorm16x4:
               componentCount = 4;
               type = GL_UNSIGNED_SHORT;
               normalized = GL_TRUE;
               break;
          }
     }


private:

     std::vector<VertexDataInfo> m_meshes;
};