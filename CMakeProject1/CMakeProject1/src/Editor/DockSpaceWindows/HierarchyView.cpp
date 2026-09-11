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

     int index = 0;

     for (const auto& gameObjectPtr :
          m_TargetScene->GetAllGameObjects())
     {
          GameObject& gameObject = *gameObjectPtr;

          // 判断当前物体是不是被选中的物体
          bool selected =
               m_ChosenIndex != nullptr &&
               *m_ChosenIndex == index;

          // 点击 Selectable 会返回 true
          if (ImGui::Selectable(
               gameObject.name.c_str(),
               selected))
          {
               // 用户点击了这个 GameObject
               if (m_ChosenIndex != nullptr)
               {
                    *m_ChosenIndex = index;
               }
          }

          index++;
     }

     ImGui::End();
}