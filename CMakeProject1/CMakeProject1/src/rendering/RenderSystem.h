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

#include "Engine/GameObject.h"
#include "Engine/Transform.h"
#include "Engine/Renderer.h"
#include "VertexDataController.h"


// ============================================================
// GPU Light Structures
// ============================================================

struct alignas(16) GPUPointLight
{
     glm::vec4 position;
     glm::vec4 colorIntensity;
     glm::vec4 params;
};

struct alignas(16) GPUSpotLight
{
     glm::vec4 position;
     glm::vec4 direction;
     glm::vec4 colorIntensity;
     glm::vec4 params;
};

struct alignas(16) GPUDirectionalLight
{
     glm::vec4 direction;
     glm::vec4 colorIntensity;
};


// ============================================================
// GPU Light Block
// ============================================================

struct GPULightBlock
{
     GPUPointLight pointLights[10];
     GPUSpotLight spotLights[10];
     GPUDirectionalLight directionalLights[10];

     glm::vec4 ambientColor;

     int pointLightCount;
     int spotLightCount;
     int directionalLightCount;
     int padding;
};


// ============================================================
// Camera Data
// ============================================================

struct CameraData
{
     glm::mat4 View;
     glm::mat4 Projection;
     glm::mat4 ViewProjection;

     glm::vec3 Position;
     unsigned int targetFramebuffer;
};


struct GPUCameraData
{
     glm::mat4 View;
     glm::mat4 Projection;
     glm::mat4 ViewProjection;

     glm::vec4 Position;
};


// ============================================================
// RenderSystem
// ============================================================

class RenderSystem
{
public:

     // --------------------------------------------------------
     // Constants
     // --------------------------------------------------------

     static constexpr int MaxPointLights = 10;
     static constexpr int MaxSpotLights = 10;
     static constexpr int MaxDirectionalLights = 10;


     // --------------------------------------------------------
     // Construction / Destruction
     // --------------------------------------------------------

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
     GLuint m_LightUBO = 0;


     // ========================================================
     // CPU-side Light Data
     // ========================================================

     GPULightBlock m_Lights{};


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
          // Light UBO
          // ----------------------------------------------------

          glGenBuffers(
               1,
               &m_LightUBO
          );

          glBindBuffer(
               GL_UNIFORM_BUFFER,
               m_LightUBO
          );

          glBufferData(
               GL_UNIFORM_BUFFER,
               sizeof(GPULightBlock),
               &m_Lights,
               GL_DYNAMIC_DRAW
          );

          glBindBufferBase(
               GL_UNIFORM_BUFFER,
               1,
               m_LightUBO
          );


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


          if (m_LightUBO != 0)
          {
               glDeleteBuffers(
                    1,
                    &m_LightUBO
               );

               m_LightUBO = 0;
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


     // ========================================================
     // Point Light
     // ========================================================

     int UpdatePointLightData(
          int index,
          const Vector3& position,
          const Vector3& color,
          float intensity,
          float range
     )
     {
          // ----------------------------------------------------
          // Allocate new light
          // ----------------------------------------------------

          if (index == -1)
          {
               if (m_Lights.pointLightCount >= MaxPointLights)
                    return -1;

               index = m_Lights.pointLightCount++;
          }
          else
          {
               if (
                    index < 0 ||
                    index >= m_Lights.pointLightCount
                    )
               {
                    return -1;
               }
          }


          // ----------------------------------------------------
          // CPU data
          // ----------------------------------------------------

          GPUPointLight& light =
               m_Lights.pointLights[index];


          light.position = glm::vec4(
               position.x,
               position.y,
               position.z,
               1.0f
          );


          light.colorIntensity = glm::vec4(
               color.x,
               color.y,
               color.z,
               intensity
          );


          light.params = glm::vec4(
               range,
               0.0f,
               0.0f,
               0.0f
          );


          // ----------------------------------------------------
          // GPU update
          // ----------------------------------------------------

          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    pointLights
               ) +
               sizeof(GPUPointLight) * index,

               sizeof(GPUPointLight),

               &light
          );


          // Count changed when allocating a new light.
          //
          // Upload it as well.
          if (index == m_Lights.pointLightCount - 1)
          {
               UpdateLightRange(
                    offsetof(
                         GPULightBlock,
                         pointLightCount
                    ),

                    sizeof(int),

                    &m_Lights.pointLightCount
               );
          }


          return index;
     }


     // ========================================================
     // Spot Light
     // ========================================================

     int UpdateSpotLightData(
          int index,
          const Vector3& position,
          const Vector3& direction,
          const Vector3& color,
          float intensity,
          float innerCone,
          float outerCone,
          float range
     )
     {
          // ----------------------------------------------------
          // Allocate new light
          // ----------------------------------------------------

          if (index == -1)
          {
               if (m_Lights.spotLightCount >= MaxSpotLights)
                    return -1;

               index = m_Lights.spotLightCount++;
          }
          else
          {
               if (
                    index < 0 ||
                    index >= m_Lights.spotLightCount
                    )
               {
                    return -1;
               }
          }


          // ----------------------------------------------------
          // CPU data
          // ----------------------------------------------------

          GPUSpotLight& light =
               m_Lights.spotLights[index];


          light.position = glm::vec4(
               position.x,
               position.y,
               position.z,
               1.0f
          );


          light.direction = glm::vec4(
               direction.x,
               direction.y,
               direction.z,
               0.0f
          );


          light.colorIntensity = glm::vec4(
               color.x,
               color.y,
               color.z,
               intensity
          );


          light.params = glm::vec4(
               innerCone,
               outerCone,
               range,
               0.0f
          );


          // ----------------------------------------------------
          // GPU update
          // ----------------------------------------------------

          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    spotLights
               ) +
               sizeof(GPUSpotLight) * index,

               sizeof(GPUSpotLight),

               &light
          );


          // Upload count if a new light was allocated.
          if (index == m_Lights.spotLightCount - 1)
          {
               UpdateLightRange(
                    offsetof(
                         GPULightBlock,
                         spotLightCount
                    ),

                    sizeof(int),

                    &m_Lights.spotLightCount
               );
          }


          return index;
     }


     // ========================================================
     // Ambient Light
     // ========================================================
     void UpdateAmbientLightData(const glm::vec3& color)
     {
          m_Lights.ambientColor = glm::vec4(color, 1.0f);

          std::size_t offset = offsetof(GPULightBlock, ambientColor);

          UpdateLightRange(
               offset,
               sizeof(glm::vec4),
               &m_Lights.ambientColor
          );
     }


     // ========================================================
     // Camera Rendering
     // ========================================================
     void RenderForCamera(const Camera& cam)
     {
          // ============================================================
          // 1. Camera GameObject
          // ============================================================

          if (cam.gameObject == nullptr)
               return;


          // ============================================================
          // 2. Camera Transform
          // ============================================================

          Vector3 position =
               cam.gameObject
               ->transform
               ->GetPosition();

          Quaternion rotation =
               cam.gameObject
               ->transform
               ->GetRotation();


          // ============================================================
          // 3. Camera GLM Transform
          // ============================================================

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


          // ============================================================
          // 4. Projection
          // ============================================================

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


          // ============================================================
          // 5. Camera GPU Data
          // ============================================================

          GPUCameraData gpuCamera{};

          gpuCamera.View = view;

          gpuCamera.Projection = projection;

          gpuCamera.ViewProjection =
               projection *
               view;

          gpuCamera.Position =
               glm::vec4(
                    glmPosition,
                    1.0f
               );


          // ============================================================
          // 6. Framebuffer
          // ============================================================

          GLint previousFBO = 0;

          glGetIntegerv(
               GL_DRAW_FRAMEBUFFER_BINDING,
               &previousFBO
          );


          SetFrameBuffer(
               cam.renderInfo_targetFramebuffer
          );


          // ============================================================
          // 7. Viewport
          // ============================================================

          GLint viewport[4]{};

          glGetIntegerv(
               GL_VIEWPORT,
               viewport
          );


          // ============================================================
          // 8. Upload Camera UBO
          // ============================================================

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


          // ============================================================
          // 9. Bind UBOs
          // ============================================================

          glBindBufferBase(
               GL_UNIFORM_BUFFER,
               0,
               m_CameraUBO
          );

          glBindBufferBase(
               GL_UNIFORM_BUFFER,
               1,
               m_LightUBO
          );


          // ============================================================
          // 10. Clear
          // ============================================================

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


          // ============================================================
          // 11. Render Objects
          // ============================================================

          for (Renderer* rd : m_Renderers)
          {
               if (rd == nullptr)
                    continue;

               if (rd->gameObject == nullptr)
                    continue;

               if (rd->renderSystemIndex < 0)
                    continue;


               // --------------------------------------------------------
               // Transform
               // --------------------------------------------------------

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


               // --------------------------------------------------------
               // Model Matrix
               // --------------------------------------------------------

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


               // ========================================================
               // Shader
               // ========================================================

               rd->shader.use();


               rd->shader.setMat4(
                    "u_Model",
                    model
               );


               rd->shader.setVec4(
                    "u_Color",
                    1.0f,
                    1.0f,
                    1.0f,
                    1.0f
               );


               // ========================================================
               // Mesh
               // ========================================================

               m_VertexDataController.useMesh(
                    rd->renderSystemIndex
               );

               m_VertexDataController.drawMesh(
                    rd->renderSystemIndex
               );
          }


          // ============================================================
          // Restore FBO
          // ============================================================

          glBindFramebuffer(
               GL_DRAW_FRAMEBUFFER,
               static_cast<GLuint>(previousFBO)
          );


          // ============================================================
          // Restore viewport
          // ============================================================

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
               // In this case use the camera's normal dimensions
               // elsewhere or leave the existing viewport intact.
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


public:

     // ========================================================
     // Ambient Light Setter
     // ========================================================

     void SetAmbientColor(
          const glm::vec3& color
     )
     {
          m_Lights.ambientColor =
               glm::vec4(
                    color,
                    1.0f
               );


          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    ambientColor
               ),

               sizeof(glm::vec4),

               &m_Lights.ambientColor
          );
     }


     // ========================================================
     // Point Light Setter
     // ========================================================

     void SetPointLight(
          int index,
          const GPUPointLight& light
     )
     {
          if (
               index < 0 ||
               index >= MaxPointLights
               )
          {
               return;
          }


          m_Lights.pointLights[index] =
               light;


          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    pointLights
               ) +
               sizeof(GPUPointLight) * index,

               sizeof(GPUPointLight),

               &light
          );
     }


     // ========================================================
     // Spot Light Setter
     // ========================================================

     void SetSpotLight(
          int index,
          const GPUSpotLight& light
     )
     {
          if (
               index < 0 ||
               index >= MaxSpotLights
               )
          {
               return;
          }


          m_Lights.spotLights[index] =
               light;


          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    spotLights
               ) +
               sizeof(GPUSpotLight) * index,

               sizeof(GPUSpotLight),

               &light
          );
     }


     // ========================================================
     // Directional Light Setter
     // ========================================================

     void SetDirectionalLight(
          int index,
          const GPUDirectionalLight& light
     )
     {
          if (
               index < 0 ||
               index >= MaxDirectionalLights
               )
          {
               return;
          }


          m_Lights.directionalLights[index] =
               light;


          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    directionalLights
               ) +
               sizeof(GPUDirectionalLight) * index,

               sizeof(GPUDirectionalLight),

               &light
          );
     }


     // ========================================================
     // Point Light Count
     // ========================================================

     void SetPointLightCount(
          int count
     )
     {
          count =
               Clamp(
                    count,
                    0,
                    MaxPointLights
               );


          m_Lights.pointLightCount =
               count;


          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    pointLightCount
               ),

               sizeof(int),

               &m_Lights.pointLightCount
          );
     }


     // ========================================================
     // Spot Light Count
     // ========================================================

     void SetSpotLightCount(
          int count
     )
     {
          count =
               Clamp(
                    count,
                    0,
                    MaxSpotLights
               );


          m_Lights.spotLightCount =
               count;


          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    spotLightCount
               ),

               sizeof(int),

               &m_Lights.spotLightCount
          );
     }


     // ========================================================
     // Directional Light Count
     // ========================================================

     void SetDirectionalLightCount(
          int count
     )
     {
          count =
               Clamp(
                    count,
                    0,
                    MaxDirectionalLights
               );


          m_Lights.directionalLightCount =
               count;


          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    directionalLightCount
               ),

               sizeof(int),

               &m_Lights.directionalLightCount
          );
     }


private:

     // ========================================================
     // Update Light UBO
     // ========================================================

     void UpdateLightRange(
          std::size_t offset,
          std::size_t size,
          const void* data
     )
     {
          if (m_LightUBO == 0)
               return;

          if (data == nullptr)
               return;


          glBindBuffer(
               GL_UNIFORM_BUFFER,
               m_LightUBO
          );


          glBufferSubData(
               GL_UNIFORM_BUFFER,
               static_cast<GLintptr>(offset),
               static_cast<GLsizeiptr>(size),
               data
          );


          glBindBuffer(
               GL_UNIFORM_BUFFER,
               0
          );
     }


     // ========================================================
     // Integer Clamp
     // ========================================================

     static int Clamp(
          int value,
          int minValue,
          int maxValue
     )
     {
          return std::max(
               minValue,
               std::min(
                    value,
                    maxValue
               )
          );
     }
};

