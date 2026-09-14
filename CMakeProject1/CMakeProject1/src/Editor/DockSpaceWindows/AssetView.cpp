#include "AssetView.h"
#include "imgui.h"

void AssetView::Draw()
{
     ImGui::Begin("Asset");


     ImGui::Text(
          "Path: %s",
          m_CurrentPath.string().c_str()
     );


     ImGui::Separator();


     ImGui::BeginChild(
          "FolderTree",
          ImVec2(200, 0),
          true
     );


     DrawFolderTree(
          m_AssetRoot
     );


     ImGui::EndChild();

     ImGui::SameLine();


     ImGui::BeginChild(
          "FileView",
          ImVec2(0, 0),
          true
     );


     DrawCurrentFolder();


     ImGui::EndChild();

     ImGui::End();
}


std::optional<int> AssetView::ReadObjectID(
     const std::filesystem::path& path)
{
     std::ifstream file(
          path
     );


     if (!file.is_open())
     {
          return std::nullopt;
     }


     // ============================================================
     // Read Entire File
     // ============================================================

     std::stringstream buffer;

     buffer <<
          file.rdbuf();


     const std::string content =
          buffer.str();


     // ============================================================
     // Find ReferenceInfo
     // ============================================================

     size_t referencePos =
          content.find(
               "\"ReferenceInfo\""
          );


     if (
          referencePos ==
          std::string::npos
          )
     {
          return std::nullopt;
     }


     // ============================================================
     // Find ObjectID after ReferenceInfo
     // ============================================================

     size_t pos =
          content.find(
               "\"ObjectID\"",
               referencePos
          );


     if (
          pos ==
          std::string::npos
          )
     {
          return std::nullopt;
     }


     // ============================================================
     // Find :
     // ============================================================

     pos =
          content.find(
               ':',
               pos
          );


     if (
          pos ==
          std::string::npos
          )
     {
          return std::nullopt;
     }


     ++pos;


     // ============================================================
     // Skip Whitespace
     // ============================================================

     while (
          pos < content.size() &&
          std::isspace(
               static_cast<unsigned char>(
                    content[pos]
                    )
          )
          )
     {
          ++pos;
     }


     // ============================================================
     // Negative
     // ============================================================

     bool negative =
          false;


     if (
          pos < content.size() &&
          content[pos] == '-'
          )
     {
          negative = true;

          ++pos;
     }


     // ============================================================
     // Must Have Number
     // ============================================================

     if (
          pos >= content.size() ||
          !std::isdigit(
               static_cast<unsigned char>(
                    content[pos]
                    )
          )
          )
     {
          return std::nullopt;
     }


     // ============================================================
     // Read Integer
     // ============================================================

     int id = 0;


     while (
          pos < content.size() &&
          std::isdigit(
               static_cast<unsigned char>(
                    content[pos]
                    )
          )
          )
     {
          id =
               id * 10 +
               (
                    content[pos] -
                    '0'
                    );

          ++pos;
     }


     if (negative)
     {
          id =
               -id;
     }


     return id;
}