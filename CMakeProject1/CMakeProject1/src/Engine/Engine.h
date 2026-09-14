#pragma once

#include <utility>

#include "Scene.h"
#include "rendering/VertexDataController.h"
#include "rendering/RenderSystem.h"

class Engine
{
public:
     Engine()
          : vertexDataController(),
          renderSystem(vertexDataController)
     {}

public:
     Scene currentScene;
     std::vector<Scene> allScene;
     OpenGLVertexDataController vertexDataController;
     RenderSystem renderSystem;
};