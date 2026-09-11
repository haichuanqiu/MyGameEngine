#pragma once
#include "Engine/Scene.h"

class HierarchyView
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

     // 不拥有这个 int，只保存外部 int 的地址
     int* m_ChosenIndex = nullptr;
};