
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

          engine.currentScene->filePath = scenePath;
          SceneSerializer::LoadScene(
               engine.currentScene,
               sceneData
          );
          engine.currentScene->filePath = scenePath;


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


     
     }
}
