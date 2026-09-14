
#pragma once

#include "Engine/Engine.h"
#include "Engine/GameObjectSystem.h"
#include "Engine/Scene.h"

#include "rendering/VertexDataController.h"

#include "Serialization/SceneSerializer.h"

#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include "Assets/AssetManager.h"

namespace testLoadScene
{
     

     Mesh RegisterMesh()
     {
          float squareVertices[] = {
               // 前面 (z = +0.5)
               -0.5f, -0.5f,  0.5f,
                0.5f, -0.5f,  0.5f,
                0.5f,  0.5f,  0.5f,
               -0.5f,  0.5f,  0.5f,

               // 后面 (z = -0.5)
               -0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f,  0.5f, -0.5f,
               -0.5f,  0.5f, -0.5f
          };

          unsigned int indices[] = {
               // 前面
               0, 1, 2,
               2, 3, 0,

               // 后面
               4, 6, 5,
               6, 4, 7,

               // 左面
               4, 0, 3,
               3, 7, 4,

               // 右面
               1, 5, 6,
               6, 2, 1,

               // 上面
               3, 2, 6,
               6, 7, 3,

               // 下面
               4, 5, 1,
               1, 0, 4
          };

          Mesh squareMesh;

          squareMesh.vertexData.resize(
               sizeof(squareVertices)
          );

          std::memcpy(
               squareMesh.vertexData.data(),
               squareVertices,
               sizeof(squareVertices)
          );

          squareMesh.indexData.resize(
               sizeof(indices)
          );

          std::memcpy(
               squareMesh.indexData.data(),
               indices,
               sizeof(indices)
          );

          squareMesh.vertexLayout.stride[0] =
               3 * sizeof(float);

          squareMesh.vertexLayout.attributes.push_back({
              VertexSemantic::Position,
              VertexFormat::Float3,
              0,
              0
               });

          squareMesh.vertexCount = 8;
          squareMesh.indexCount = 36;

          return squareMesh;
     }


     // ============================================================
     // Load Scene
     // ============================================================

     void OnSceneCreated(Engine& engine)
     {
          // ============================================================
               // Mesh
               // ============================================================
          const std::string assetPath =
               "../../../../CMakeProject1/assets";


          AssetManager::Scan(assetPath);
          std::cout
               << "====================================\n"
               << "Start Load Scene\n"
               << "====================================\n";


          // ========================================================
          // 1. 重新注册运行时 Mesh
          // ========================================================

          Mesh cubeMesh =
               RegisterMesh();

          int cubeMeshID =
               engine.vertexDataController.registerMesh(
                    cubeMesh
               );

          std::cout
               << "Registered Cube Mesh. ID = "
               << cubeMeshID
               << std::endl;


          // ========================================================
          // 2. 读取 Scene 文件
          // ========================================================

          const std::string scenePath =
               "../../../../CMakeProject1/assets/SceneData";

          std::ifstream file(
               scenePath,
               std::ios::in
          );

          if (!file.is_open())
          {
               std::cerr
                    << "Failed to open Scene file: "
                    << scenePath
                    << std::endl;

               return;
          }

          std::stringstream buffer;

          buffer << file.rdbuf();

          file.close();

          const std::string sceneData =
               buffer.str();


          std::cout
               << "Scene file loaded.\n";


          // ========================================================
          // 3. Load Scene
          // ========================================================

          SceneSerializer::LoadScene(
               &engine.currentScene,
               sceneData
          );


          // ========================================================
          // 4. 检查加载结果
          // ========================================================

          std::cout
               << "\n====================================\n"
               << "Loaded Scene\n"
               << "====================================\n";

          std::cout
               << "GameObject Count = "
               << engine.currentScene
               .GetAllGameObjects()
               .size()
               << std::endl;


          // ========================================================
          // 5. 恢复运行时状态
          // ========================================================

          for (
               const auto& gameObjectPtr :
               engine.currentScene.GetAllGameObjects()
               )
          {
               GameObject& gameObject =
                    *gameObjectPtr;


               std::cout
                    << "\nGameObject: "
                    << gameObject.name
                    << std::endl;


               const auto& components =
                    gameObject.GetComponents();


               std::cout
                    << "Component Count = "
                    << components.size()
                    << std::endl;


               for (
                    const auto& componentPtr :
                    components
                    )
               {
                    Component* component =
                         componentPtr.get();


                    const TypeInfo* typeInfo =
                         ReflectionRegistry::Instance()
                         .Find(
                              typeid(*component)
                         );


                    if (!typeInfo)
                    {
                         std::cout
                              << "  Unknown Component"
                              << std::endl;

                         continue;
                    }


                    std::cout
                         << "  Component = "
                         << typeInfo->name
                         << std::endl;


                    std::cout
                         << "  TypeId = "
                         << typeInfo->id
                         << std::endl;


                    // =================================================
                    // Transform
                    // =================================================

                    if (
                         auto* transform =
                         dynamic_cast<Transform*>(
                              component
                              )
                         )
                    {
                         std::cout
                              << "    Transform loaded"
                              << std::endl;
                    }


                    // =================================================
                    // Renderer
                    // =================================================

                    if (
                         auto* renderer =
                         dynamic_cast<Renderer*>(
                              component
                              )
                         )
                    {
                         std::cout
                              << "    Renderer loaded"
                              << std::endl;

                         std::cout
                              << "    renderSystemIndex = "
                              << renderer->renderSystemIndex
                              << std::endl;
                         renderer->material->setShader();

                         // Scene 中保存的是 Mesh ID。
                         //
                         // 这里假设重新注册 Mesh 后，
                         // cubeMeshID 与保存时一致。

                         if (
                              renderer->renderSystemIndex
                              != cubeMeshID
                              )
                         {
                              std::cerr
                                   << "WARNING: Renderer mesh ID "
                                   << renderer->renderSystemIndex
                                   << " != current mesh ID "
                                   << cubeMeshID
                                   << std::endl;
                         }
                         

                         // 重新注册 Renderer 到运行时 RenderSystem

                         engine.renderSystem.RegisterRenderer( renderer);
                    }


                    // =================================================
                    // PointLight
                    // =================================================

                    if (
                         auto* light =
                         dynamic_cast<PointLight*>(
                              component
                              )
                         )
                    {
                         std::cout
                              << "    PointLight loaded"
                              << std::endl;

                         std::cout
                              << "    Intensity = "
                              << light->Intensity
                              << std::endl;

                         std::cout
                              << "    Range = "
                              << light->Range
                              << std::endl;


                         // RenderSystem 是运行时对象，
                         // 不从 Scene 文件恢复。

                         light->SetDefaultTarget(
                              &engine.renderSystem
                         );


                         light->UpdateLightData(
                              engine.renderSystem
                         );
                    }
               }
          }


          // ========================================================
          // 6. Ambient Light
          // ========================================================

          engine.renderSystem.GetLighting().UpdateAmbientLightData(
               glm::vec3(
                    0.2f,
                    0.2f,
                    0.2f
               )
          );


          // ========================================================
          // 7. 最终结果
          // ========================================================

          std::cout
               << "\n====================================\n"
               << "Scene Load Finished\n"
               << "====================================\n";


          for (
               const auto& gameObjectPtr :
               engine.currentScene.GetAllGameObjects()
               )
          {
               GameObject& gameObject =
                    *gameObjectPtr;


               std::cout
                    << gameObject.name
                    << " -> "
                    << gameObject
                    .GetComponents()
                    .size()
                    << " components"
                    << std::endl;
          }
     }
}
