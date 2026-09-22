#pragma once

#include <utility>
#include <unordered_map>

#include "Scene.h"
#include "rendering/VertexDataController.h"
#include "rendering/RenderSystem.h"
#include "Profiler/profiler.h"
class Engine
{
public:

     static Engine& Instance()
     {
          static Engine instance;
          return instance;
     }


     // ============================================================
     // Get Scene
     // ============================================================

     Scene* GetScene(
          int sceneID)
     {
          auto it =
               allScene.find(
                    sceneID
               );


          if (it == allScene.end())
               return nullptr;


          return it->second.get();
     }


     const Scene* GetScene(
          int sceneID) const
     {
          auto it =
               allScene.find(
                    sceneID
               );


          if (it == allScene.end())
               return nullptr;


          return it->second.get();
     }


     // ============================================================
     // Get Current Scene
     // ============================================================

     Scene* GetCurrentScene()
     {
          return currentScene;
     }


     const Scene* GetCurrentScene() const
     {
          return currentScene;
     }


     // ============================================================
     // Update
     // ============================================================

     void Update()
     {
          if (!currentScene)
               return;


          std::vector<Component*> components;


          {
               ENGINE_PROFILE_SCOPE("Find Components");


               components =
                    currentScene
                    ->FindAllOfType<Component>();
          }


          {
               ENGINE_PROFILE_SCOPE("Component Update");


               for (Component* component :
                    components)
               {
                    if (!component)
                         continue;


                    component->Update();
               }
          }
     }


public:

     // ============================================================
     // Scene Storage
     //
     // Engine owns every Scene.
     // ============================================================

     std::unordered_map<
          int,
          std::unique_ptr<Scene>
     > allScene;


     // ============================================================
     // Current Scene
     //
     // Non-owning pointer.
     // Actual ownership is in allScene.
     // ============================================================

     Scene* currentScene =
          nullptr;


     OpenGLVertexDataController vertexDataController;

     RenderSystem renderSystem;


private:

     Engine()
          : vertexDataController(),
          renderSystem(
               vertexDataController
          )
     {
          // ========================================================
          // Create Default Scene
          // ========================================================

          const int sceneID = 0;


          auto scene =
               std::make_unique<Scene>();


          scene->sceneIndex =
               sceneID;


          currentScene =
               scene.get();


          allScene[
               sceneID
          ] =
               std::move(scene);
     }


     Engine(
          const Engine&) = delete;


     Engine& operator=(
          const Engine&) = delete;
};