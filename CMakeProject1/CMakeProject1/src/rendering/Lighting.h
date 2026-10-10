#pragma once

#include <glad/glad.h>

#include <algorithm>
#include <cstddef>
#include <unordered_map>

#include "Utils/IDGenerator.h"

// 按你项目实际路径 include
// #include "LightData.h"
// #include "Vector3.h"


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
          std::cout << "End lighting" << std::endl;

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


     // ========================================================
     // Remove Point Light
     //
     // target = Random Light ID
     // ========================================================

     bool RemovePointLight(int target)
     {
          auto targetIt =
               m_PointLightIDToSlot.find(target);

          if (
               targetIt ==
               m_PointLightIDToSlot.end()
               )
          {
               return false;
          }


          const int removedSlot =
               targetIt->second;

          const int lastSlot =
               m_Lights.pointLightCount - 1;


          // ===================================================
          // If this isn't the last light,
          // move the last light into the removed slot.
          // ===================================================

          if (removedSlot != lastSlot)
          {
               m_Lights.pointLights[removedSlot] =
                    m_Lights.pointLights[lastSlot];


               // Get the ID belonging to the last slot.

               const int movedLightID =
                    m_PointLightSlotToID[lastSlot];


               // Update ID -> Slot mapping.

               m_PointLightIDToSlot[movedLightID] =
                    removedSlot;


               // Update Slot -> ID mapping.

               m_PointLightSlotToID[removedSlot] =
                    movedLightID;


               // Upload moved light.

               UpdateLightRange(
                    offsetof(
                         GPULightBlock,
                         pointLights
                    ) +
                    sizeof(GPUPointLight) *
                    removedSlot,

                    sizeof(GPUPointLight),

                    &m_Lights.pointLights[
                         removedSlot
                    ]
               );
          }


          // ===================================================
          // Clear old last slot
          // ===================================================

          m_Lights.pointLights[lastSlot] =
               GPUPointLight{};


          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    pointLights
               ) +
               sizeof(GPUPointLight) *
               lastSlot,

               sizeof(GPUPointLight),

               &m_Lights.pointLights[
                    lastSlot
               ]
          );


          // ===================================================
          // Remove ID mappings
          // ===================================================

          m_PointLightIDToSlot.erase(
               targetIt
          );

          m_PointLightSlotToID.erase(
               lastSlot
          );


          // ===================================================
          // Decrease count
          // ===================================================

          --m_Lights.pointLightCount;


          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    pointLightCount
               ),

               sizeof(int),

               &m_Lights.pointLightCount
          );


          return true;
     }


     // ========================================================
     // Update Point Light
     //
     // lightID == -1
     //     Create new light and generate random ID.
     //
     // lightID != -1
     //     Find existing light using random ID.
     //
     // Return:
     //     Random Light ID
     //
     //     -1 = Failed
     // ========================================================

     int UpdatePointLightData(
          int lightID,
          const Vector3& position,
          const Vector3& color,
          float intensity,
          float range
     )
     {
          int slot = -1;


          // ===================================================
          // Create
          // ===================================================

          if (lightID == -1)
          {
               if (
                    m_Lights.pointLightCount >=
                    MaxPointLights
                    )
               {
                    return -1;
               }


               lightID =
                    IDGenerator::GenerateObjectID(
                         [this](int id)
                         {
                              return
                                   IsLightIDUsed(id);
                         }
                    );


               slot =
                    m_Lights.pointLightCount;


               ++m_Lights.pointLightCount;


               // ID -> Slot

               m_PointLightIDToSlot[
                    lightID
               ] = slot;


               // Slot -> ID

               m_PointLightSlotToID[
                    slot
               ] = lightID;


               // Upload new count.

               UpdateLightRange(
                    offsetof(
                         GPULightBlock,
                         pointLightCount
                    ),

                    sizeof(int),

                    &m_Lights.pointLightCount
               );
          }

          // ===================================================
          // Update Existing
          // ===================================================

          else
          {
               auto it =
                    m_PointLightIDToSlot.find(
                         lightID
                    );


               if (
                    it ==
                    m_PointLightIDToSlot.end()
                    )
               {
                    return -1;
               }


               slot =
                    it->second;
          }


          // ===================================================
          // Update CPU data
          // ===================================================

          GPUPointLight& light =
               m_Lights.pointLights[slot];


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


          // ===================================================
          // Upload GPU data
          // ===================================================

          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    pointLights
               ) +
               sizeof(GPUPointLight) * slot,

               sizeof(GPUPointLight),

               &light
          );


          return lightID;
     }


     // ========================================================
     // Ambient Light Data
     // ========================================================

     void UpdateAmbientLightData(
          const glm::vec3& color
     )
     {
          m_Lights.ambientColor =
               glm::vec4(
                    color,
                    1.0f
               );


          std::size_t offset =
               offsetof(
                    GPULightBlock,
                    ambientColor
               );


          UpdateLightRange(
               offset,
               sizeof(glm::vec4),
               &m_Lights.ambientColor
          );
     }


     // ========================================================
     // Remove Spot Light
     //
     // target = Random Light ID
     // ========================================================

     bool RemoveSpotLight(int target)
     {
          auto targetIt =
               m_SpotLightIDToSlot.find(target);


          if (
               targetIt ==
               m_SpotLightIDToSlot.end()
               )
          {
               return false;
          }


          const int removedSlot =
               targetIt->second;

          const int lastSlot =
               m_Lights.spotLightCount - 1;


          // ===================================================
          // Move last light into removed slot
          // ===================================================

          if (removedSlot != lastSlot)
          {
               m_Lights.spotLights[removedSlot] =
                    m_Lights.spotLights[lastSlot];


               const int movedLightID =
                    m_SpotLightSlotToID[
                         lastSlot
                    ];


               m_SpotLightIDToSlot[
                    movedLightID
               ] = removedSlot;


               m_SpotLightSlotToID[
                    removedSlot
               ] = movedLightID;


               UpdateLightRange(
                    offsetof(
                         GPULightBlock,
                         spotLights
                    ) +
                    sizeof(GPUSpotLight) *
                    removedSlot,

                    sizeof(GPUSpotLight),

                    &m_Lights.spotLights[
                         removedSlot
                    ]
               );
          }


          // ===================================================
          // Clear last slot
          // ===================================================

          m_Lights.spotLights[lastSlot] =
               GPUSpotLight{};


          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    spotLights
               ) +
               sizeof(GPUSpotLight) *
               lastSlot,

               sizeof(GPUSpotLight),

               &m_Lights.spotLights[
                    lastSlot
               ]
          );


          // ===================================================
          // Remove mapping
          // ===================================================

          m_SpotLightIDToSlot.erase(
               targetIt
          );

          m_SpotLightSlotToID.erase(
               lastSlot
          );


          // ===================================================
          // Count
          // ===================================================

          --m_Lights.spotLightCount;


          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    spotLightCount
               ),

               sizeof(int),

               &m_Lights.spotLightCount
          );


          return true;
     }


     // ========================================================
     // Update Spot Light
     //
     // lightID == -1 -> Create
     // otherwise     -> Update
     //
     // Returns random Light ID
     // ========================================================

     int UpdateSpotLightData(
          int lightID,
          const Vector3& position,
          const Vector3& direction,
          const Vector3& color,
          float intensity,
          float innerCone,
          float outerCone,
          float range
     )
     {
          int slot = -1;


          // ===================================================
          // Create
          // ===================================================

          if (lightID == -1)
          {
               if (
                    m_Lights.spotLightCount >=
                    MaxSpotLights
                    )
               {
                    return -1;
               }


               lightID =
                    IDGenerator::GenerateObjectID(
                         [this](int id)
                         {
                              return
                                   IsLightIDUsed(id);
                         }
                    );


               slot =
                    m_Lights.spotLightCount;


               ++m_Lights.spotLightCount;


               // ID -> Slot

               m_SpotLightIDToSlot[
                    lightID
               ] = slot;


               // Slot -> ID

               m_SpotLightSlotToID[
                    slot
               ] = lightID;


               // Upload count.

               UpdateLightRange(
                    offsetof(
                         GPULightBlock,
                         spotLightCount
                    ),

                    sizeof(int),

                    &m_Lights.spotLightCount
               );
          }

          // ===================================================
          // Update Existing
          // ===================================================

          else
          {
               auto it =
                    m_SpotLightIDToSlot.find(
                         lightID
                    );


               if (
                    it ==
                    m_SpotLightIDToSlot.end()
                    )
               {
                    return -1;
               }


               slot =
                    it->second;
          }


          // ===================================================
          // Update CPU data
          // ===================================================

          GPUSpotLight& light =
               m_Lights.spotLights[slot];


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


          // ===================================================
          // Upload GPU data
          // ===================================================

          UpdateLightRange(
               offsetof(
                    GPULightBlock,
                    spotLights
               ) +
               sizeof(GPUSpotLight) * slot,

               sizeof(GPUSpotLight),

               &light
          );


          return lightID;
     }


private:

     GLuint m_LightUBO = 0;

     GPULightBlock m_Lights{};


     // ========================================================
     // Point Light ID Mapping
     //
     // Random ID -> GPU Slot
     // GPU Slot  -> Random ID
     // ========================================================

     std::unordered_map<int, int>
          m_PointLightIDToSlot;

     std::unordered_map<int, int>
          m_PointLightSlotToID;


     // ========================================================
     // Spot Light ID Mapping
     // ========================================================

     std::unordered_map<int, int>
          m_SpotLightIDToSlot;

     std::unordered_map<int, int>
          m_SpotLightSlotToID;


private:

     // ========================================================
     // Check Light ID
     //
     // Point / Spot share the same ID namespace.
     // ========================================================

     bool IsLightIDUsed(int id) const
     {
          if (
               m_PointLightIDToSlot.find(id) !=
               m_PointLightIDToSlot.end()
               )
          {
               return true;
          }


          if (
               m_SpotLightIDToSlot.find(id) !=
               m_SpotLightIDToSlot.end()
               )
          {
               return true;
          }


          return false;
     }


     // ========================================================
     // Update GPU Range
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
               static_cast<GLintptr>(
                    offset
                    ),
               static_cast<GLsizeiptr>(
                    size
                    ),
               data
          );


          glBindBuffer(
               GL_UNIFORM_BUFFER,
               0
          );
     }


     // ========================================================
     // Clamp
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