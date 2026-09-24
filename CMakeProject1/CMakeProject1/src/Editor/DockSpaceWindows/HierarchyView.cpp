#include "HierarchyView.h"
#include "imgui.h"
#include "Engine/Scene.h"



void HierarchyView::Draw()
{
     ImGui::Begin("Hierarchy");

     if (ImGui::IsWindowHovered() &&
          ImGui::IsMouseClicked(ImGuiMouseButton_Right))
     {
          ImVec2 mousePos =
               ImGui::GetMousePos();

          ImVec2 windowPos =
               ImGui::GetWindowPos();

          ImVec2 localPos(
               mousePos.x - windowPos.x,
               mousePos.y - windowPos.y
          );
          OnRightClickedEmptySpace.Invoke(Vector2(localPos.x, localPos.y));
     }
     // ============================================================
     // Save Button
     // ============================================================

     if (ImGui::Button("Save"))
     {
          OnSaveClicked.Invoke(m_TargetScene);
     }


     ImGui::Separator();


     // ============================================================
     // No Scene
     // ============================================================

     if (m_TargetScene == nullptr)
     {
          ImGui::TextDisabled(
               "No Scene"
          );

          ImGui::End();

          return;
     }


     // ============================================================
     // GameObjects
     // ============================================================

     for (
          const auto& gameObjectPtr :
          m_TargetScene->GetAllGameObjects()
          )
     {
          if (!gameObjectPtr)
               continue;


          GameObject& gameObject =
               *gameObjectPtr;


          if (
               ImGui::Selectable(
                    gameObject.name.c_str()
               )
               )
          {
               OnGameObjectClicked.Invoke(
                    &gameObject
               );
          }
     }


     ImGui::End();
}