#pragma once
#pragma once

#include <algorithm>
#include <cstddef>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GPUDataShapes.h"


class Lighting
{
public:

     static constexpr int MaxPointLights = 10;
     static constexpr int MaxSpotLights = 10;
     static constexpr int MaxDirectionalLights = 10;


public:

     Lighting()
     {
          Init();
     }


     ~Lighting()
     {
          Shutdown();
     }


     Lighting(const Lighting&) = delete;
     Lighting& operator=(const Lighting&) = delete;

     Lighting(Lighting&&) = delete;
     Lighting& operator=(Lighting&&) = delete;


public:

     // ========================================================
     // Initialization
     // ========================================================

     void Init()
     {
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
          if (m_LightUBO != 0)
          {
               glDeleteBuffers(
                    1,
                    &m_LightUBO
               );

               m_LightUBO = 0;
          }
     }


     // ========================================================
     // Ambient
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
     // Point Light
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
     // Spot Light
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
     // Directional Light
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
     // Counts
     // ========================================================

     void SetPointLightCount(int count)
     {
          count = Clamp(
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


     void SetSpotLightCount(int count)
     {
          count = Clamp(
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


     void SetDirectionalLightCount(int count)
     {
          count = Clamp(
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


     // ========================================================
     // Binding
     // ========================================================

     void Bind()
     {
          glBindBufferBase(
               GL_UNIFORM_BUFFER,
               1,
               m_LightUBO
          );
     }
     int UpdatePointLightData(
          int index,
          const Vector3& position,
          const Vector3& color,
          float intensity,
          float range
     )
     {
          if (index == -1)
          {
               if (m_Lights.pointLightCount >= MaxPointLights)
                    return -1;

               index =
                    m_Lights.pointLightCount++;
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

          GPUPointLight& light =
               m_Lights.pointLights[index];

          light.position =
               glm::vec4(
                    position.x,
                    position.y,
                    position.z,
                    1.0f
               );

          light.colorIntensity =
               glm::vec4(
                    color.x,
                    color.y,
                    color.z,
                    intensity
               );

          light.params =
               glm::vec4(
                    range,
                    0.0f,
                    0.0f,
                    0.0f
               );

          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    pointLights
               ) +
               sizeof(GPUPointLight) * index,

               sizeof(GPUPointLight),

               &light
          );

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
          if (index == -1)
          {
               if (m_Lights.spotLightCount >= MaxSpotLights)
                    return -1;

               index =
                    m_Lights.spotLightCount++;
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

          GPUSpotLight& light =
               m_Lights.spotLights[index];

          light.position =
               glm::vec4(
                    position.x,
                    position.y,
                    position.z,
                    1.0f
               );

          light.direction =
               glm::vec4(
                    direction.x,
                    direction.y,
                    direction.z,
                    0.0f
               );

          light.colorIntensity =
               glm::vec4(
                    color.x,
                    color.y,
                    color.z,
                    intensity
               );

          light.params =
               glm::vec4(
                    innerCone,
                    outerCone,
                    range,
                    0.0f
               );

          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    spotLights
               ) +
               sizeof(GPUSpotLight) * index,

               sizeof(GPUSpotLight),

               &light
          );

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

private:

     GLuint m_LightUBO = 0;

     GPULightBlock m_Lights{};


private:

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