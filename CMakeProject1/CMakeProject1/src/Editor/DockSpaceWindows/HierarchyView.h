#pragma once

#include "Engine/Scene.h"
#include "Event.h"

class HierarchyView
{
public:

     void SetTarget(Scene* scene)
     {
          m_TargetScene = scene;
     }

     void Draw();

     Event<GameObject*> OnGameObjectClicked;

private:

     Scene* m_TargetScene = nullptr;
     int* m_ChosenIndex = nullptr;
};