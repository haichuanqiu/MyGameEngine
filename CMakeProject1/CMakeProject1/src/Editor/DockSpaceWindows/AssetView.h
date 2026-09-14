#pragma once

#include <imgui.h>

#include <cctype>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

#include "Event.h" // 改成你自己的 Event 路径

#pragma once



class AssetView
{
public:

     // ============================================================
     // Event
     // ============================================================

     // 当一个 .AssetObject 被点击，并且成功读取到
     // ReferenceInfo.ObjectID
     // 时触发
     Event<int> OnObjectIDClicked;


     AssetView()
     {
          m_AssetRoot =
               std::filesystem::current_path()
               / "../../../../CMakeProject1";

          m_AssetRoot =
               std::filesystem::weakly_canonical(
                    m_AssetRoot
               );

          m_CurrentPath = m_AssetRoot;
     }


     void Draw();


private:

     std::filesystem::path m_AssetRoot;
     std::filesystem::path m_CurrentPath;


     // ============================================================
     // 从 .AssetObject 中读取
     //
     // "ObjectID": 123
     //
     // 只寻找这个字段。
     // ============================================================

     std::optional<int> ReadObjectID(
          const std::filesystem::path& path);


     // ============================================================
     // 左边 Folder Tree
     // ============================================================

     void DrawFolderTree(
          const std::filesystem::path& parentPath)
     {
          for (
               const auto& entry :
               std::filesystem::directory_iterator(
                    parentPath
               )
               )
          {
               if (!entry.is_directory())
               {
                    continue;
               }


               const auto folderPath =
                    entry.path();

               std::string folderName =
                    folderPath.filename().string();


               bool isSelected =
                    folderPath == m_CurrentPath;


               bool hasChildren = false;

               for (
                    const auto& child :
                    std::filesystem::directory_iterator(
                         folderPath
                    )
                    )
               {
                    if (child.is_directory())
                    {
                         hasChildren = true;
                         break;
                    }
               }


               if (hasChildren)
               {
                    ImGuiTreeNodeFlags flags =
                         ImGuiTreeNodeFlags_OpenOnArrow |
                         ImGuiTreeNodeFlags_OpenOnDoubleClick;


                    if (isSelected)
                    {
                         flags |=
                              ImGuiTreeNodeFlags_Selected;
                    }


                    bool open =
                         ImGui::TreeNodeEx(
                              folderName.c_str(),
                              flags
                         );


                    if (ImGui::IsItemClicked())
                    {
                         m_CurrentPath =
                              folderPath;
                    }


                    if (open)
                    {
                         DrawFolderTree(
                              folderPath
                         );

                         ImGui::TreePop();
                    }
               }
               else
               {
                    ImGuiTreeNodeFlags flags =
                         ImGuiTreeNodeFlags_Leaf |
                         ImGuiTreeNodeFlags_NoTreePushOnOpen;


                    if (isSelected)
                    {
                         flags |=
                              ImGuiTreeNodeFlags_Selected;
                    }


                    ImGui::TreeNodeEx(
                         folderName.c_str(),
                         flags
                    );


                    if (ImGui::IsItemClicked())
                    {
                         m_CurrentPath =
                              folderPath;
                    }
               }
          }
     }


     // ============================================================
     // 右边当前文件夹
     // ============================================================

     void DrawCurrentFolder()
     {
          for (
               const auto& entry :
               std::filesystem::directory_iterator(
                    m_CurrentPath
               )
               )
          {
               const auto path =
                    entry.path();

               std::string name =
                    path.filename().string();


               if (entry.is_directory())
               {
                    if (
                         ImGui::Selectable(
                              (
                                   "[Folder] " +
                                   name
                                   ).c_str()
                         )
                         )
                    {
                         m_CurrentPath = path;
                    }

                    continue;
               }


               if (
                    path.extension() ==
                    ".AssetObject"
                    )
               {
                    if (
                         ImGui::Selectable(
                              (
                                   "[Asset] " +
                                   name
                                   ).c_str()
                         )
                         )
                    {
                         std::optional<int> id =
                              ReadObjectID(
                                   path
                              );


                         if (id.has_value())
                         {
                              OnObjectIDClicked.Invoke(
                                   *id
                              );
                         }
                    }

                    continue;
               }


               ImGui::Text(
                    "[File] %s",
                    name.c_str()
               );
          }
     }
};   