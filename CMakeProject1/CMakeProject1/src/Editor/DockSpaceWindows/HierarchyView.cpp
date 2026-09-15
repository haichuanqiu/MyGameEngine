#include "HierarchyView.h"
#include "imgui.h"
#include "Engine/Scene.h"



void HierarchyView::Draw()
{
     ImGui::Begin("Hierarchy");


     // ============================================================
     // Save Button
     // ============================================================

     if (ImGui::Button("Save"))
     {
          OnSaveClicked.Invoke();
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