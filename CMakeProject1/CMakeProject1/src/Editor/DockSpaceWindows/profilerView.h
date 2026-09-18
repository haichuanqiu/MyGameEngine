#pragma once
#pragma once

#include "imgui.h"
#include "Profiler/profiler.h"

#include <algorithm>
#include <vector>


class ProfilerView
{
public:

     void Draw()
     {
          ImGui::Begin("Profiler");

          DrawRecordingControl();

          ImGui::Separator();

          DrawCurrentFrame();

          ImGui::End();
     }


private:

     void DrawRecordingControl()
     {
          auto& profiler =
               ProfilerManager::Instance();


          // ----------------------------------------
          // Recording Status
          // ----------------------------------------

          if (profiler.IsRecording())
          {
               ImGui::Text("Status: Recording");

               ImGui::SameLine();

               if (ImGui::Button("Stop"))
               {
                    profiler.StopRecording();
               }
          }
          else
          {
               ImGui::Text("Status: Stopped");

               ImGui::SameLine();

               if (ImGui::Button("Start"))
               {
                    profiler.StartRecording();
               }
          }


          ImGui::SameLine();

          if (ImGui::Button("Clear"))
          {
               profiler.Clear();
          }
     }


     void DrawCurrentFrame()
     {
          const auto& frame =
               ProfilerManager::Instance()
               .GetCurrentFrame();


          if (frame.empty())
          {
               ImGui::Text("No profiling data.");
               return;
          }


          // Profiler 的 Event 是析构的时候提交的，
          // 所以原始顺序不一定是开始执行的顺序。
          //
          // 这里复制一份，然后按照 startMs 排序。
          std::vector<ProfileEvent> sortedEvents =
               frame;


          std::sort(
               sortedEvents.begin(),
               sortedEvents.end(),
               [](const ProfileEvent& a,
                    const ProfileEvent& b)
               {
                    return a.startMs < b.startMs;
               }
          );


          // ----------------------------------------
          // Table
          // ----------------------------------------

          if (ImGui::BeginTable(
               "ProfilerTable",
               3,
               ImGuiTableFlags_Borders |
               ImGuiTableFlags_RowBg |
               ImGuiTableFlags_Resizable))
          {
               ImGui::TableSetupColumn(
                    "Section",
                    ImGuiTableColumnFlags_WidthStretch
               );

               ImGui::TableSetupColumn(
                    "Start (ms)",
                    ImGuiTableColumnFlags_WidthFixed,
                    100.0f
               );

               ImGui::TableSetupColumn(
                    "Duration (ms)",
                    ImGuiTableColumnFlags_WidthFixed,
                    120.0f
               );


               ImGui::TableHeadersRow();


               // ----------------------------------------
               // Events
               // ----------------------------------------

               for (const auto& event : sortedEvents)
               {
                    ImGui::TableNextRow();


                    // ====================================
                    // Name
                    // ====================================

                    ImGui::TableSetColumnIndex(0);


                    // 根据 depth 做缩进
                    const float indentAmount =
                         static_cast<float>(event.depth) * 15.0f;


                    ImGui::Indent(indentAmount);

                    ImGui::TextUnformatted(
                         event.name.c_str()
                    );

                    ImGui::Unindent(indentAmount);


                    // ====================================
                    // Start
                    // ====================================

                    ImGui::TableSetColumnIndex(1);

                    ImGui::Text(
                         "%.3f",
                         event.startMs
                    );


                    // ====================================
                    // Duration
                    // ====================================

                    ImGui::TableSetColumnIndex(2);

                    ImGui::Text(
                         "%.3f",
                         event.durationMs
                    );
               }


               ImGui::EndTable();
          }
     }
};