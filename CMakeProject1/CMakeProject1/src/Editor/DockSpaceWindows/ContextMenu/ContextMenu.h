#pragma once

#include "imgui.h"

#include "HierarchyViewContextMenu.h"
// #include "AssetViewContextMenu.h"


class ContextMenu
{
public:

     enum class OpenType
     {
          None,

          Hierarchy,
          AssetView
     };


public:

     // ========================================================
     // Open
     // ========================================================

     void Open(
          OpenType type,
          Vector2 position)
     {
          m_OpenType = type;
          m_OpenPosition = ImVec2(position.x, position.y);

          m_ShouldOpen = true;
     }


     // ========================================================
     // Draw
     // ========================================================

     void Draw()
     {
          // ----------------------------------------------------
          // 有 View 请求打开 Context Menu
          // ----------------------------------------------------

          if (m_ShouldOpen)
          {
               ImGui::OpenPopup("EditorContextMenu");

               m_ShouldOpen = false;
          }


          // ----------------------------------------------------
          // Popup
          // ----------------------------------------------------

          if (ImGui::BeginPopup("EditorContextMenu"))
          {
               switch (m_OpenType)
               {
               case OpenType::Hierarchy:
               {
                    ContextMenuImplementation::
                         DrawHierarchyViewContextMenu();

                    break;
               }


               case OpenType::AssetView:
               {
                    // ContextMenuImplementation::
                    //     DrawAssetViewContextMenu();

                    break;
               }


               case OpenType::None:
               default:
                    break;
               }


               ImGui::EndPopup();
          }
     }


private:

     OpenType m_OpenType =
          OpenType::None;


     ImVec2 m_OpenPosition =
          ImVec2(0.0f, 0.0f);


     bool m_ShouldOpen =
          false;
};