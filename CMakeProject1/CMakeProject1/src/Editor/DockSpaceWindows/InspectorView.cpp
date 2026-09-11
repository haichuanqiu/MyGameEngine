#include "InspectorView.h"

#include "imgui.h"

#include "Engine/GameObjectSystem.h"
#include "Serialization/Reflection.h"
#include "Editor/DataInEditor/ClassInspector.h"

#include "Serialization/JsonSerializer.h"
void InspectorView::Draw()
{
     ImGui::Begin("Inspector");


     // ========================================================
     // Check Scene
     // ========================================================

     if (m_TargetScene == nullptr ||
          m_ChosenIndex == nullptr)
     {
          ImGui::End();
          return;
     }


     int index =
          *m_ChosenIndex;


     // ========================================================
     // Check GameObject
     // ========================================================

     if (index < 0 ||
          index >= static_cast<int>(
               m_TargetScene
               ->GetAllGameObjects()
               .size()))
     {
          ImGui::Text(
               "No GameObject Selected"
          );

          ImGui::End();
          return;
     }


     // ========================================================
     // Get GameObject
     // ========================================================

     GameObject& gameObject =
          m_TargetScene->GetGameObject(index);


     // ========================================================
     // GameObject Name
     // ========================================================

     ImGui::Text(
          "Name: %s",
          gameObject.name.c_str()
     );

     ImGui::Separator();


     // ========================================================
     // Components
     // ========================================================

     const auto& components =
          gameObject.GetComponents();


     for (size_t i = 0;
          i < components.size();
          ++i)
     {
          const auto& component =
               components[i];


          if (!component)
               continue;


          // ----------------------------------------------------
          // 防止不同 Component 之间 ImGui ID 冲突
          // ----------------------------------------------------

          ImGui::PushID(static_cast<int>(i));
          ClassInspector inspector("Component", *component);
          inspector.Draw();
          ImGui::PopID();


          ImGui::Separator();
     }

     ImGui::End();
}