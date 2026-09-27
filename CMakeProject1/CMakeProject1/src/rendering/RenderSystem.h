
#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Engine/Components/RenderingRelatedComponents.h"
#include "VertexDataController.h"

#include "rendering/GPUDataShapes.h"
#include "rendering/Lighting.h"


// ============================================================
// RenderSystem
// ============================================================

class RenderSystem
{
public:

     // --------------------------------------------------------
     // Construction / Destruction
     // --------------------------------------------------------
     Lighting& GetLighting()
     {
          return m_Lighting;
     }

     explicit RenderSystem(
          OpenGLVertexDataController& vertexDataController
     )
          : m_VertexDataController(vertexDataController)
     {
          Init();
     }


     ~RenderSystem()
     {
          Shutdown();
     }


     // RenderSystem owns OpenGL resources.
     // Do not copy or move it.
     RenderSystem(const RenderSystem&) = delete;
     RenderSystem& operator=(const RenderSystem&) = delete;

     RenderSystem(RenderSystem&&) = delete;
     RenderSystem& operator=(RenderSystem&&) = delete;


private:

     // ========================================================
     // OpenGL Resources
     // ========================================================

     GLuint m_CameraUBO = 0;


     // ========================================================
     // Lighting
     // ========================================================

     Lighting m_Lighting;


     // ========================================================
     // External Dependencies
     // ========================================================

     // RenderSystem does NOT own this.
     //
     // Engine owns OpenGLVertexDataController.
     // RenderSystem only references it.
     //
     OpenGLVertexDataController& m_VertexDataController;


     // ========================================================
     // Registered Renderers
     // ========================================================

     std::vector<Renderer*> m_Renderers;


     // ========================================================
     // Initialization
     // ========================================================

     void Init()
     {
          // ----------------------------------------------------
          // Camera UBO
          // ----------------------------------------------------

          glGenBuffers(
               1,
               &m_CameraUBO
          );

          glBindBuffer(
               GL_UNIFORM_BUFFER,
               m_CameraUBO
          );

          glBufferData(
               GL_UNIFORM_BUFFER,
               sizeof(GPUCameraData),
               nullptr,
               GL_DYNAMIC_DRAW
          );

          glBindBufferBase(
               GL_UNIFORM_BUFFER,
               0,
               m_CameraUBO
          );


          // ----------------------------------------------------
          // Lighting
          // ----------------------------------------------------

          m_Lighting.Bind();


          // ----------------------------------------------------
          // Unbind
          // ----------------------------------------------------

          glBindBuffer(
               GL_UNIFORM_BUFFER,
               0
          );
     }


     // ========================================================
     // Shutdown
     // ========================================================

     void Shutdown()
     {
          if (m_CameraUBO != 0)
          {
               glDeleteBuffers(
                    1,
                    &m_CameraUBO
               );

               m_CameraUBO = 0;
          }
     }


public:

     // ========================================================
     // Renderer Registration
     // ========================================================

     bool HasRenderer(
          Renderer* renderer
     ) const
     {
          if (renderer == nullptr)
               return false;

          return std::find(
               m_Renderers.begin(),
               m_Renderers.end(),
               renderer
          ) != m_Renderers.end();
     }


     bool RegisterRenderer(
          Renderer* renderer
     )
     {
          if (renderer == nullptr)
               return false;

          if (HasRenderer(renderer))
               return false;

          m_Renderers.push_back(renderer);

          return true;
     }


     bool RemoveRenderer(
          Renderer* renderer
     )
     {
          if (renderer == nullptr)
               return false;

          auto it = std::find(
               m_Renderers.begin(),
               m_Renderers.end(),
               renderer
          );

          if (it == m_Renderers.end())
               return false;

          m_Renderers.erase(it);

          return true;
     }



     const Lighting& GetLighting() const
     {
          return m_Lighting;
     }


     // ========================================================
     // Camera Rendering
     // ========================================================

     void RenderForCamera(
          const Camera& cam
     )
     {
          // ====================================================
          // 1. Camera GameObject
          // ====================================================

          if (cam.gameObject == nullptr)
               return;


          // ====================================================
          // 2. Camera Transform
          // ====================================================

          Vector3 position =
               cam.gameObject
               ->transform
               ->GetPosition();

          Quaternion rotation =
               cam.gameObject
               ->transform
               ->GetRotation();


          // ====================================================
          // 3. Camera GLM Transform
          // ====================================================

          glm::vec3 glmPosition(
               position.x,
               position.y,
               position.z
          );

          glm::quat glmRotation(
               rotation.w,
               rotation.x,
               rotation.y,
               rotation.z
          );


          glm::mat4 world =
               glm::translate(
                    glm::mat4(1.0f),
                    glmPosition
               ) *
               glm::mat4_cast(
                    glmRotation
               );


          glm::mat4 view =
               glm::inverse(world);


          // ====================================================
          // 4. Projection
          // ====================================================

          float aspect = 1.0f;

          if (cam.Height > 0)
          {
               aspect =
                    static_cast<float>(cam.Width) /
                    static_cast<float>(cam.Height);
          }


          glm::mat4 projection =
               glm::perspective(
                    glm::radians(cam.FieldOfView),
                    aspect,
                    cam.NearClip,
                    cam.FarClip
               );


          // ====================================================
          // 5. Camera GPU Data
          // ====================================================

          GPUCameraData gpuCamera{};

          gpuCamera.View =
               view;

          gpuCamera.Projection =
               projection;

          gpuCamera.ViewProjection =
               projection *
               view;

          gpuCamera.Position =
               glm::vec4(
                    glmPosition,
                    1.0f
               );


          // ====================================================
          // 6. Framebuffer
          // ====================================================

          GLint previousFBO = 0;

          glGetIntegerv(
               GL_DRAW_FRAMEBUFFER_BINDING,
               &previousFBO
          );


          SetFrameBuffer(
               cam.renderInfo_targetFramebuffer
          );


          // ====================================================
          // 7. Viewport
          // ====================================================

          GLint viewport[4]{};

          glGetIntegerv(
               GL_VIEWPORT,
               viewport
          );


          // ====================================================
          // 8. Upload Camera UBO
          // ====================================================

          glBindBuffer(
               GL_UNIFORM_BUFFER,
               m_CameraUBO
          );

          glBufferSubData(
               GL_UNIFORM_BUFFER,
               0,
               sizeof(GPUCameraData),
               &gpuCamera
          );

          glBindBuffer(
               GL_UNIFORM_BUFFER,
               0
          );


          // ====================================================
          // 9. Bind UBOs
          // ====================================================

          glBindBufferBase(
               GL_UNIFORM_BUFFER,
               0,
               m_CameraUBO
          );

          m_Lighting.Bind();


          // ====================================================
          // 10. Clear
          // ====================================================

          glClearColor(
               0.1f,
               0.1f,
               0.1f,
               1.0f
          );

          glClear(
               GL_COLOR_BUFFER_BIT |
               GL_DEPTH_BUFFER_BIT
          );


          // ====================================================
          // 11. Render Objects
          // ====================================================

          for (Renderer* rd : m_Renderers)
          {
               if (rd == nullptr)
                    continue;

               if (rd->gameObject == nullptr)
                    continue;
               auto vertexDataObject= rd->vertexData.get();
               if (!vertexDataObject || vertexDataObject->RenderSystemIndex < 0)
                    continue;

               if (rd->material == nullptr)
                    continue;

               // ------------------------------------------------
               // Transform
               // ------------------------------------------------

               Vector3 objectPosition =
                    rd->gameObject
                    ->transform
                    ->GetPosition();

               Quaternion objectRotation =
                    rd->gameObject
                    ->transform
                    ->GetRotation();

               Vector3 objectScale =
                    rd->gameObject
                    ->transform
                    ->GetScale();


               // ------------------------------------------------
               // Model Matrix
               // ------------------------------------------------

               glm::mat4 model(1.0f);


               model =
                    glm::translate(
                         model,
                         glm::vec3(
                              objectPosition.x,
                              objectPosition.y,
                              objectPosition.z
                         )
                    );


               glm::quat objectQuat(
                    objectRotation.w,
                    objectRotation.x,
                    objectRotation.y,
                    objectRotation.z
               );


               model *=
                    glm::mat4_cast(
                         objectQuat
                    );


               model =
                    glm::scale(
                         model,
                         glm::vec3(
                              objectScale.x,
                              objectScale.y,
                              objectScale.z
                         )
                    );


               // =================================================
               // Shader
               // =================================================

               rd->material->shader->use();

               rd->material->shader->setMat4(
                    "u_Model",
                    model
               );

               rd->material->shader->setVec4(
                    "u_Color",
                    1.0f,
                    1.0f,
                    1.0f,
                    1.0f
               );


               // =================================================
               // Mesh
               // =================================================

               m_VertexDataController.useMesh(
                    vertexDataObject->RenderSystemIndex
               );

               m_VertexDataController.drawMesh(
                    vertexDataObject->RenderSystemIndex
               );
          }


          // ====================================================
          // Restore FBO
          // ====================================================

          glBindFramebuffer(
               GL_DRAW_FRAMEBUFFER,
               static_cast<GLuint>(previousFBO)
          );


          // ====================================================
          // Restore viewport
          // ====================================================

          glViewport(
               viewport[0],
               viewport[1],
               viewport[2],
               viewport[3]
          );
     }


private:

     // ========================================================
     // Set Framebuffer
     // ========================================================

     void SetFrameBuffer(
          GLuint targetFramebuffer
     )
     {
          glBindFramebuffer(
               GL_DRAW_FRAMEBUFFER,
               targetFramebuffer
          );


          // ----------------------------------------------------
          // Default framebuffer
          // ----------------------------------------------------

          if (targetFramebuffer == 0)
          {
               // For the default framebuffer, we cannot query
               // the texture attachment.
               //
               // Leave the existing viewport intact.
               return;
          }


          GLint fboWidth = 0;
          GLint fboHeight = 0;

          GLint type = GL_NONE;


          // ----------------------------------------------------
          // Query color attachment
          // ----------------------------------------------------

          glGetFramebufferAttachmentParameteriv(
               GL_DRAW_FRAMEBUFFER,
               GL_COLOR_ATTACHMENT0,
               GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
               &type
          );


          // ----------------------------------------------------
          // Texture attachment
          // ----------------------------------------------------

          if (type == GL_TEXTURE)
          {
               GLint texture = 0;


               glGetFramebufferAttachmentParameteriv(
                    GL_DRAW_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                    &texture
               );


               GLint previousTexture = 0;


               glGetIntegerv(
                    GL_TEXTURE_BINDING_2D,
                    &previousTexture
               );


               glBindTexture(
                    GL_TEXTURE_2D,
                    static_cast<GLuint>(texture)
               );


               glGetTexLevelParameteriv(
                    GL_TEXTURE_2D,
                    0,
                    GL_TEXTURE_WIDTH,
                    &fboWidth
               );


               glGetTexLevelParameteriv(
                    GL_TEXTURE_2D,
                    0,
                    GL_TEXTURE_HEIGHT,
                    &fboHeight
               );


               glBindTexture(
                    GL_TEXTURE_2D,
                    static_cast<GLuint>(previousTexture)
               );
          }


          // ----------------------------------------------------
          // Renderbuffer attachment
          // ----------------------------------------------------

          else if (type == GL_RENDERBUFFER)
          {
               GLint renderbuffer = 0;


               glGetFramebufferAttachmentParameteriv(
                    GL_DRAW_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                    &renderbuffer
               );


               GLint previousRenderbuffer = 0;


               glGetIntegerv(
                    GL_RENDERBUFFER_BINDING,
                    &previousRenderbuffer
               );


               glBindRenderbuffer(
                    GL_RENDERBUFFER,
                    static_cast<GLuint>(renderbuffer)
               );


               glGetRenderbufferParameteriv(
                    GL_RENDERBUFFER,
                    GL_RENDERBUFFER_WIDTH,
                    &fboWidth
               );


               glGetRenderbufferParameteriv(
                    GL_RENDERBUFFER,
                    GL_RENDERBUFFER_HEIGHT,
                    &fboHeight
               );


               glBindRenderbuffer(
                    GL_RENDERBUFFER,
                    static_cast<GLuint>(previousRenderbuffer)
               );
          }


          // ----------------------------------------------------
          // Set viewport only if valid
          // ----------------------------------------------------

          if (
               fboWidth > 0 &&
               fboHeight > 0
               )
          {
               glViewport(
                    0,
                    0,
                    fboWidth,
                    fboHeight
               );
          }
     }
};
