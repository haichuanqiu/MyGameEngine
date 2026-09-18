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


     Scene* GetScene(int sceneID)
     {
          auto it = allScene.find(sceneID);

          if (it == allScene.end())
               return nullptr;

          return it->second;
     }


     const Scene* GetScene(int sceneID) const
     {
          auto it = allScene.find(sceneID);

          if (it == allScene.end())
               return nullptr;

          return it->second;
     }

     void Update()
     {

          std::vector<Component*> components;

          {
               ENGINE_PROFILE_SCOPE("Find Components");

               components =
                    currentScene.FindAllOfType<Component>();
          }

          {
               ENGINE_PROFILE_SCOPE("Component Update");

               for (auto component : components)
               {
                    if (component)
                    {
                         component->Update();
                    }
               }
          }
     }
public:

     Scene currentScene;

     std::unordered_map<int, Scene*> allScene;

     OpenGLVertexDataController vertexDataController;
     RenderSystem renderSystem;

private:

     Engine()
          : vertexDataController(),
          renderSystem(vertexDataController)
     {
          currentScene.sceneIndex = 0;

          allScene[0] = &currentScene;
     }


     Engine(const Engine&) = delete;
     Engine& operator=(const Engine&) = delete;
};