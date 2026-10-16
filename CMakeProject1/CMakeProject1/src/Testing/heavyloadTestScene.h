#pragma once
#include "Engine/Components/preformanceTest.h"

#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "Engine/Components/preformanceTest.h"

#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "Engine/Components/preformanceTest.h"

namespace heavyloadTestScene
{
     void OnSceneCreated(Engine& engine)
     {
          // ============================================================
          // Test Settings
          // ============================================================

          constexpr int OBJECT_COUNT = 10000;
          constexpr int Iteration = 1;
          constexpr int GRID_WIDTH = 100;
          constexpr float OBJECT_SPACING = 1.5f;
          constexpr int CONTROLLER_COUNT = 5;

          // ============================================================
          // Enable / Disable Each Test
          //
          // true  = Create Controller + Objects
          // false = Do Not Create Controller Or Objects
          // ============================================================

          constexpr bool ENABLE_01_SINGLE_VECTORBOOL = true;

          constexpr bool ENABLE_02_MT_VECTORBOOL_ARRAY = true;

          constexpr bool ENABLE_03_MT_UINT8_ARRAY = true;

          constexpr bool ENABLE_04_MT_VECTORBOOL_TRANSFORMARRAY = true;

          constexpr bool ENABLE_05_MT_UINT8_TRANSFORMARRAY = true;

          std::cout
               << "Start creating 5-version swarm test scene..."
               << std::endl;

          // ============================================================
          // Light
          // ============================================================

          GameObject& light =
               engine.currentScene->AddGameObject();

          light.name = "Light";

          light.transform->SetPosition(
               Vector3(
                    2.0f,
                    10.0f,
                    2.0f
               )
          );

          PointLight* lightComponent =
               light.AddComponent<PointLight>();

          lightComponent->Intensity = 10.0f;

          lightComponent->UpdateLightData(
               engine.renderSystem
          );

          lightComponent->SetDefaultTarget(
               &engine.renderSystem
          );

          engine.renderSystem
               .GetLighting()
               .UpdateAmbientLightData(
                    glm::vec3(
                         0.2f,
                         0.2f,
                         0.2f
                    )
               );

          // ============================================================
          // Shared Material
          // ============================================================

          Material material;

          material.setVertShaderPath(
               "../../../../CMakeProject1/assets/shaders/vert.vs"
          );

          material.setFragShaderPath(
               "../../../../CMakeProject1/assets/shaders/frag.fs"
          );

          int materialID =
               AssetManager::Instance().Register(
                    std::move(material)
               );

          Material* sharedMaterial =
               AssetManager::Instance().Find<Material>(
                    materialID
               );

          if (
               sharedMaterial == nullptr ||
               sharedMaterial->shader == nullptr
               )
          {
               std::cout
                    << "Failed to create material!"
                    << std::endl;

               return;
          }

          // ============================================================
          // Shared Vertices
          // ============================================================

          Vertices vertices;

          vertices.SetPreset(0);

          int verticesID =
               AssetManager::Instance().Register(
                    std::move(vertices)
               );

          Vertices* sharedVertices =
               AssetManager::Instance().Find<Vertices>(
                    verticesID
               );

          if (sharedVertices == nullptr)
          {
               std::cout
                    << "Failed to create vertices!"
                    << std::endl;

               return;
          }

          // ============================================================
          // Material Settings
          // ============================================================

          sharedMaterial->shader->setVec4(
               "u_Color",
               1.0f,
               1.0f,
               1.0f,
               1.0f
          );

          // ============================================================
          // Controller Positions
          //
          // 5 independent groups along X axis
          // ============================================================

          const Vector3 controllerPositions[CONTROLLER_COUNT] =
          {
              Vector3(-400.0f, 0.0f, -40.0f),
              Vector3(-200.0f, 0.0f, -40.0f),
              Vector3(0.0f, 0.0f, -40.0f),
              Vector3(200.0f, 0.0f, -40.0f),
              Vector3(400.0f, 0.0f, -40.0f)
          };

          const std::string controllerNames[CONTROLLER_COUNT] =
          {
              "01_Single_VectorBool",
              "02_MT_VectorBool_Array",
              "03_MT_Uint8_Array",
              "04_MT_VectorBool_TransformArray",
              "05_MT_Uint8_TransformArray"
          };

          // ============================================================
          // Controller Pointers
          //
          // Disabled Controllers Remain nullptr
          // ============================================================

          Swarm_Single_VectorBool* swarm1 = nullptr;

          Swarm_MT_VectorBool_Array* swarm2 = nullptr;

          Swarm_MT_Uint8_Array* swarm3 = nullptr;

          Swarm_MT_VectorBool_TransformArray* swarm4 = nullptr;

          Swarm_MT_Uint8_TransformArray* swarm5 = nullptr;

          // ============================================================
          // Shared Controller Settings
          //
          // All Controllers Use Identical Parameters
          // ============================================================

          auto ConfigureSwarm = [&](auto* swarm)
               {
                    swarm->radius = 20.0f;

                    swarm->moveSpeed = 1.0f;

                    swarm->waveStrength = 5.0f;

                    swarm->spacing = 0.05f;

                    swarm->calculationIterations = Iteration;

                    swarm->objects.reserve(OBJECT_COUNT);
               };

          // ============================================================
          // Controller 01
          //
          // Single Thread + vector<bool> + Normal Array
          // ============================================================

          if (ENABLE_01_SINGLE_VECTORBOOL)
          {
               GameObject& controller =
                    engine.currentScene->AddGameObject();

               controller.name =
                    controllerNames[0];

               controller.transform->SetPosition(
                    controllerPositions[0]
               );

               swarm1 =
                    controller.AddComponent<
                    Swarm_Single_VectorBool
                    >();

               ConfigureSwarm(swarm1);
          }

          // ============================================================
          // Controller 02
          //
          // Multi Thread + vector<bool> + Normal Array
          // ============================================================

          if (ENABLE_02_MT_VECTORBOOL_ARRAY)
          {
               GameObject& controller =
                    engine.currentScene->AddGameObject();

               controller.name =
                    controllerNames[1];

               controller.transform->SetPosition(
                    controllerPositions[1]
               );

               swarm2 =
                    controller.AddComponent<
                    Swarm_MT_VectorBool_Array
                    >();

               ConfigureSwarm(swarm2);
          }

          // ============================================================
          // Controller 03
          //
          // Multi Thread + vector<uint8_t> + Normal Array
          // ============================================================

          if (ENABLE_03_MT_UINT8_ARRAY)
          {
               GameObject& controller =
                    engine.currentScene->AddGameObject();

               controller.name =
                    controllerNames[2];

               controller.transform->SetPosition(
                    controllerPositions[2]
               );

               swarm3 =
                    controller.AddComponent<
                    Swarm_MT_Uint8_Array
                    >();

               ConfigureSwarm(swarm3);
          }

          // ============================================================
          // Controller 04
          //
          // Multi Thread + vector<bool> + TransformArray
          // ============================================================

          if (ENABLE_04_MT_VECTORBOOL_TRANSFORMARRAY)
          {
               GameObject& controller =
                    engine.currentScene->AddGameObject();

               controller.name =
                    controllerNames[3];

               controller.transform->SetPosition(
                    controllerPositions[3]
               );

               swarm4 =
                    controller.AddComponent<
                    Swarm_MT_VectorBool_TransformArray
                    >();

               ConfigureSwarm(swarm4);
          }

          // ============================================================
          // Controller 05
          //
          // Multi Thread + vector<uint8_t> + TransformArray
          // ============================================================

          if (ENABLE_05_MT_UINT8_TRANSFORMARRAY)
          {
               GameObject& controller =
                    engine.currentScene->AddGameObject();

               controller.name =
                    controllerNames[4];

               controller.transform->SetPosition(
                    controllerPositions[4]
               );

               swarm5 =
                    controller.AddComponent<
                    Swarm_MT_Uint8_TransformArray
                    >();

               ConfigureSwarm(swarm5);
          }

          // ============================================================
          // Create Objects Helper
          //
          // Each Enabled Controller Gets OBJECT_COUNT Objects
          // ============================================================

          auto CreateSwarmObjects =
               [&](auto* swarm, int groupIndex)
               {
                    const std::string& groupName =
                         controllerNames[groupIndex];

                    const Vector3& groupCenter =
                         controllerPositions[groupIndex];

                    std::cout
                         << "Creating "
                         << OBJECT_COUNT
                         << " objects for "
                         << groupName
                         << "..."
                         << std::endl;

                    for (int i = 0; i < OBJECT_COUNT; ++i)
                    {
                         // ====================================================
                         // GameObject
                         // ====================================================

                         GameObject& obj =
                              engine.currentScene->AddGameObject();

                         obj.name =
                              groupName +
                              "_SwarmObject_" +
                              std::to_string(i);

                         // ====================================================
                         // Grid Position
                         // ====================================================

                         const int xIndex =
                              i % GRID_WIDTH;

                         const int zIndex =
                              i / GRID_WIDTH;

                         const float x =
                              groupCenter.x +
                              static_cast<float>(xIndex)
                              * OBJECT_SPACING
                              - 75.0f;

                         const float z =
                              groupCenter.z +
                              static_cast<float>(zIndex)
                              * OBJECT_SPACING
                              - 20.0f;

                         obj.transform->SetPosition(
                              Vector3(
                                   x,
                                   0.0f,
                                   z
                              )
                         );

                         // ====================================================
                         // Scale
                         // ====================================================

                         obj.transform->SetScale(
                              Vector3(
                                   0.1f,
                                   0.1f,
                                   0.1f
                              )
                         );

                         // ====================================================
                         // Renderer
                         // ====================================================

                         Renderer* renderer =
                              obj.AddComponent<Renderer>();

                         // ====================================================
                         // Shared Material
                         // ====================================================

                         renderer->material =
                              sharedMaterial;

                         // ====================================================
                         // Shared Vertex Data
                         // ====================================================

                         renderer->vertexData.SetReferenceInfo(
                              sharedVertices->ReferenceInfo
                         );

                         // ====================================================
                         // EngineObjectHandle
                         // ====================================================

                         EngineObjectHandle<GameObject> handle;

                         handle.SetReferenceInfo(
                              obj.ReferenceInfo
                         );

                         // ====================================================
                         // Add To Controller
                         // ====================================================

                         swarm->objects.push_back(
                              std::move(handle)
                         );
                    }

                    std::cout
                         << "Finished "
                         << groupName
                         << " - Objects: "
                         << swarm->objects.size()
                         << std::endl;
               };

          // ============================================================
          // Create Objects Only For Enabled Controllers
          // ============================================================

          if (swarm1)
          {
               CreateSwarmObjects(swarm1, 0);
          }

          if (swarm2)
          {
               CreateSwarmObjects(swarm2, 1);
          }

          if (swarm3)
          {
               CreateSwarmObjects(swarm3, 2);
          }

          if (swarm4)
          {
               CreateSwarmObjects(swarm4, 3);
          }

          if (swarm5)
          {
               CreateSwarmObjects(swarm5, 4);
          }

          // ============================================================
          // Save Shared Assets
          // ============================================================

          AssetManager::Instance().Save<Material>(
               materialID,
               "../../../../CMakeProject1/assets/"
               "MultiThreadTest.Material.AssetObject"
          );

          AssetManager::Instance().Save<Vertices>(
               verticesID,
               "../../../../CMakeProject1/assets/"
               "MultiThreadTest.vertex.AssetObject"
          );

          // ============================================================
          // Scene Serialize
          // ============================================================

          /*
          auto s =
              SceneSerializer::Serialize(
                  *engine.currentScene
              );

          SceneSerializer::SaveSceneTo(
              *engine.currentScene,
              "../../../../CMakeProject1/assets/"
              "MultiThreadTestScene"
          );

          std::cout
              << s
              << std::endl;
          */

          // ============================================================
          // Result
          //
          // nullptr Safe
          // ============================================================

          const size_t count1 =
               swarm1 ? swarm1->objects.size() : 0;

          const size_t count2 =
               swarm2 ? swarm2->objects.size() : 0;

          const size_t count3 =
               swarm3 ? swarm3->objects.size() : 0;

          const size_t count4 =
               swarm4 ? swarm4->objects.size() : 0;

          const size_t count5 =
               swarm5 ? swarm5->objects.size() : 0;

          const size_t totalObjects =
               count1 +
               count2 +
               count3 +
               count4 +
               count5;

          const int enabledControllers =
               static_cast<int>(swarm1 != nullptr) +
               static_cast<int>(swarm2 != nullptr) +
               static_cast<int>(swarm3 != nullptr) +
               static_cast<int>(swarm4 != nullptr) +
               static_cast<int>(swarm5 != nullptr);

          std::cout
               << "========================================"
               << std::endl;

          std::cout
               << "Swarm Performance Test Scene"
               << std::endl;

          std::cout
               << "----------------------------------------"
               << std::endl;

          std::cout
               << "01 Single VectorBool: "
               << count1
               << std::endl;

          std::cout
               << "02 MT VectorBool Array: "
               << count2
               << std::endl;

          std::cout
               << "03 MT Uint8 Array: "
               << count3
               << std::endl;

          std::cout
               << "04 MT VectorBool TransformArray: "
               << count4
               << std::endl;

          std::cout
               << "05 MT Uint8 TransformArray: "
               << count5
               << std::endl;

          std::cout
               << "----------------------------------------"
               << std::endl;

          std::cout
               << "Enabled Controllers: "
               << enabledControllers
               << std::endl;

          std::cout
               << "Total Swarm Objects: "
               << totalObjects
               << std::endl;

          std::cout
               << "Calculation Iterations: "
               << Iteration
               << std::endl;

          std::cout
               << "========================================"
               << std::endl;

          std::cout
               << std::filesystem::current_path()
               << std::endl;
     }
}
