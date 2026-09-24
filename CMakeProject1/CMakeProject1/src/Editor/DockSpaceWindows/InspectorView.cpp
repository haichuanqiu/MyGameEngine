#include "InspectorView.h"

#include "imgui.h"
#include "Editor/DataInEditor/ClassInspector.h"

void InspectorView::Draw()
{
     ImGui::Begin("Inspector");

     if (!m_Inspector)
     {
          ImGui::Text("Nothing Selected");
          ImGui::End();
          return;
     }

    
     if (!m_Inspector->Draw())
     {
          ClearTarget();
          std::cout
               << "change target"
               << std::endl;
     }
     ImGui::End();
}

