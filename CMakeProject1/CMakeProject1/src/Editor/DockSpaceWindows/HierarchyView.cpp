#include "HierarchyView.h"
#include "imgui.h"
#include "Engine/Scene.h"


void HierarchyView::Draw()
{
     ImGui::Begin("Hierarchy");

     if (m_TargetScene == nullptr)
     {
          ImGui::End();
          return;
     }

     for (const auto& gameObjectPtr :
          m_TargetScene->GetAllGameObjects())
     {
          GameObject& gameObject = *gameObjectPtr;

          if (ImGui::Selectable(gameObject.name.c_str()))
          {
               // 用户点击了 GameObject

               OnGameObjectClicked.Invoke(&gameObject);
          }
     }

     ImGui::End();
}