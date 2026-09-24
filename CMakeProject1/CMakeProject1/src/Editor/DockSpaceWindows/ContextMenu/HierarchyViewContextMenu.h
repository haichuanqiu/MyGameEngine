#pragma once
#include "Editor/CommandUtility.h"
namespace ContextMenuImplementation {

     inline void DrawHierarchyViewContextMenu() {
          if (ImGui::MenuItem("Create Empty"))
          {
               CommandUtility::CreateGameObjectInCurrentScene();
          }


          if (ImGui::BeginMenu("Create"))
          {
               if (ImGui::MenuItem("Camera"))
               {
                    // Create Camera
               }


               if (ImGui::MenuItem("Light"))
               {
                    // Create Light
               }


               ImGui::EndMenu();
          }
	}
}