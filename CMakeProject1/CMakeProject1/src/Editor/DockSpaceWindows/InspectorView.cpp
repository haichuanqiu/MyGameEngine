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

     m_Inspector->Draw();

     ImGui::End();
}

