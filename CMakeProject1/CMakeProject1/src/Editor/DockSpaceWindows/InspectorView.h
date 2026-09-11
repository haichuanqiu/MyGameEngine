#pragma once
#include "Engine/GameObjectSystem.h"
#include "Engine/Scene.h"
class InspectorView
{
public:
     void SetTarget(Scene* scene)
     {
          m_TargetScene = scene;
     }

     void SetChosenIndex(int* index)
     {
          m_ChosenIndex = index;
     }

	void Draw();
private:
     Scene* m_TargetScene = nullptr;
     int* m_ChosenIndex = nullptr;
};
