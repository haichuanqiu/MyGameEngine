#pragma once

#include "imgui.h"
#include "Profiler/profiler.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <vector>


class ProfilerView
{
public:

     void Draw()
     {
          ImGui::Begin("Profiler");

          auto& profiler = ProfilerManager::Instance();

          DrawToolbar(profiler);

          ImGui::Separator();

          const auto& frames = profiler.GetFrames();

          if (frames.empty())
          {
               ImGui::TextDisabled("No profiling data.");
               ImGui::End();
               return;
          }

          ValidateSelectedFrame(frames);

          DrawFrameHistory(frames);

          ImGui::Separator();

          DrawSelectedFrameInfo(frames);

          ImGui::Separator();

          const auto& selectedFrame =
               frames[m_SelectedFrameIndex];

          DrawTimeline(selectedFrame);

          ImGui::Separator();

          DrawEventTable(selectedFrame);

          ImGui::End();
     }


private:

     // ========================================================
     // Toolbar
     // ========================================================

     void DrawToolbar(ProfilerManager& profiler)
     {
          if (profiler.IsRecording())
          {
               ImGui::Text("Recording: ON");

               ImGui::SameLine();

               if (ImGui::Button("Stop"))
               {
                    profiler.StopRecording();

                    // Stop 后停留在最后一帧
                    m_FollowLatestFrame = false;
               }
          }
          else
          {
               ImGui::Text("Recording: OFF");

               ImGui::SameLine();

               if (ImGui::Button("Start"))
               {
                    profiler.StartRecording();

                    m_FollowLatestFrame = true;
               }
          }


          ImGui::SameLine();

          if (ImGui::Button("Clear"))
          {
               profiler.Clear();

               m_SelectedFrameIndex = -1;
               m_FollowLatestFrame = true;
          }


          ImGui::SameLine();

          ImGui::Checkbox(
               "Follow Latest",
               &m_FollowLatestFrame
          );


          ImGui::SameLine();

          ImGui::Text(
               "Stored Frames: %zu",
               profiler.GetFrames().size()
          );
     }


     // ========================================================
     // Selection
     // ========================================================

     void ValidateSelectedFrame(
          const std::vector<std::vector<ProfileEvent>>& frames)
     {
          if (frames.empty())
          {
               m_SelectedFrameIndex = -1;
               return;
          }


          // 实时跟踪最新帧
          if (m_FollowLatestFrame)
          {
               m_SelectedFrameIndex =
                    static_cast<int>(frames.size()) - 1;

               return;
          }


          // 防止 Clear / frame history 滚动之后越界
          if (m_SelectedFrameIndex < 0)
          {
               m_SelectedFrameIndex = 0;
          }

          if (m_SelectedFrameIndex >=
               static_cast<int>(frames.size()))
          {
               m_SelectedFrameIndex =
                    static_cast<int>(frames.size()) - 1;
          }
     }


     // ========================================================
     // Frame History
     // ========================================================

     void DrawFrameHistory(
          const std::vector<std::vector<ProfileEvent>>& frames)
     {
          ImGui::Text("Frame History");

          const float graphHeight = 100.0f;

          const ImVec2 graphPosition =
               ImGui::GetCursorScreenPos();

          const float graphWidth =
               ImGui::GetContentRegionAvail().x;


          if (graphWidth <= 0.0f)
               return;


          // ----------------------------------------------------
          // Calculate frame times
          // ----------------------------------------------------

          std::vector<float> frameTimes;

          frameTimes.reserve(frames.size());


          float maxFrameTime = 16.67f;


          for (const auto& frame : frames)
          {
               const float frameTime =
                    CalculateFrameTime(frame);

               frameTimes.push_back(frameTime);

               maxFrameTime =
                    std::max(
                         maxFrameTime,
                         frameTime
                    );
          }


          // 留一点顶部空间
          maxFrameTime *= 1.10f;


          // ----------------------------------------------------
          // Background
          // ----------------------------------------------------

          ImDrawList* drawList =
               ImGui::GetWindowDrawList();


          const ImVec2 graphEnd(
               graphPosition.x + graphWidth,
               graphPosition.y + graphHeight
          );


          drawList->AddRect(
               graphPosition,
               graphEnd,
               ImGui::GetColorU32(
                    ImGuiCol_Border
               )
          );


          // ----------------------------------------------------
          // 16.67ms reference line
          // ----------------------------------------------------

          DrawReferenceLine(
               drawList,
               graphPosition,
               graphWidth,
               graphHeight,
               maxFrameTime,
               16.67f,
               "16.67 ms"
          );


          // ----------------------------------------------------
          // 33.33ms reference line
          // ----------------------------------------------------

          if (maxFrameTime >= 33.33f)
          {
               DrawReferenceLine(
                    drawList,
                    graphPosition,
                    graphWidth,
                    graphHeight,
                    maxFrameTime,
                    33.33f,
                    "33.33 ms"
               );
          }


          // ----------------------------------------------------
          // Frame Bars
          // ----------------------------------------------------

          const float frameWidth =
               graphWidth /
               static_cast<float>(frames.size());


          for (std::size_t i = 0;
               i < frameTimes.size();
               ++i)
          {
               const float normalized =
                    std::clamp(
                         frameTimes[i] / maxFrameTime,
                         0.0f,
                         1.0f
                    );


               const float barHeight =
                    normalized * graphHeight;


               const float x0 =
                    graphPosition.x +
                    static_cast<float>(i) * frameWidth;


               const float x1 =
                    x0 +
                    std::max(
                         1.0f,
                         frameWidth - 1.0f
                    );


               const float y0 =
                    graphPosition.y +
                    graphHeight -
                    barHeight;


               const float y1 =
                    graphPosition.y +
                    graphHeight;


               ImU32 color =
                    ImGui::GetColorU32(
                         ImGuiCol_PlotHistogram
                    );


               if (static_cast<int>(i) ==
                    m_SelectedFrameIndex)
               {
                    color =
                         ImGui::GetColorU32(
                              ImGuiCol_ButtonActive
                         );
               }


               drawList->AddRectFilled(
                    ImVec2(x0, y0),
                    ImVec2(x1, y1),
                    color
               );
          }


          // ----------------------------------------------------
          // Invisible Button
          // ----------------------------------------------------

          ImGui::InvisibleButton(
               "##ProfilerFrameHistory",
               ImVec2(
                    graphWidth,
                    graphHeight
               )
          );


          // ----------------------------------------------------
          // Mouse interaction
          // ----------------------------------------------------

          if (ImGui::IsItemHovered())
          {
               const ImVec2 mouse =
                    ImGui::GetMousePos();


               const float localX =
                    mouse.x - graphPosition.x;


               int hoveredIndex =
                    static_cast<int>(
                         localX / frameWidth
                         );


               hoveredIndex =
                    std::clamp(
                         hoveredIndex,
                         0,
                         static_cast<int>(
                              frames.size()
                              ) - 1
                    );


               ImGui::BeginTooltip();

               ImGui::Text(
                    "Frame %d",
                    hoveredIndex
               );

               ImGui::Text(
                    "%.3f ms",
                    frameTimes[hoveredIndex]
               );

               if (frameTimes[hoveredIndex] > 0.0f)
               {
                    ImGui::Text(
                         "%.1f FPS",
                         1000.0f /
                         frameTimes[hoveredIndex]
                    );
               }

               ImGui::EndTooltip();


               if (ImGui::IsMouseClicked(
                    ImGuiMouseButton_Left))
               {
                    m_SelectedFrameIndex =
                         hoveredIndex;

                    // 用户手动选择后停止自动追踪
                    m_FollowLatestFrame = false;
               }
          }
     }


     // ========================================================
     // Selected Frame Info
     // ========================================================

     void DrawSelectedFrameInfo(
          const std::vector<std::vector<ProfileEvent>>& frames)
     {
          if (m_SelectedFrameIndex < 0)
               return;


          const auto& frame =
               frames[m_SelectedFrameIndex];


          const float frameTime =
               CalculateFrameTime(frame);


          ImGui::Text(
               "Selected Frame: %d",
               m_SelectedFrameIndex
          );


          ImGui::SameLine();


          ImGui::Text(
               "| Frame Time: %.3f ms",
               frameTime
          );


          if (frameTime > 0.0f)
          {
               ImGui::SameLine();

               ImGui::Text(
                    "| %.1f FPS",
                    1000.0f / frameTime
               );
          }


          ImGui::SameLine();

          ImGui::Text(
               "| Events: %zu",
               frame.size()
          );
     }


     // ========================================================
     // Timeline
     // ========================================================

     void DrawTimeline(
          const std::vector<ProfileEvent>& frame)
     {
          ImGui::Text("Timeline");

          if (frame.empty())
          {
               ImGui::TextDisabled(
                    "No events in this frame."
               );

               return;
          }


          // --------------------------------------------------------
          // Sort
          // --------------------------------------------------------

          std::vector<ProfileEvent> events = frame;

          SortEvents(events);


          const float frameTime =
               CalculateFrameTime(events);


          if (frameTime <= 0.0f)
               return;


          // --------------------------------------------------------
          // Timeline Table
          // --------------------------------------------------------

          constexpr float rowHeight = 24.0f;


          if (ImGui::BeginTable(
               "ProfilerTimelineTable",
               2,
               ImGuiTableFlags_BordersInnerV |
               ImGuiTableFlags_RowBg |
               ImGuiTableFlags_Resizable |
               ImGuiTableFlags_SizingStretchProp))
          {
               // ====================================================
               // Columns
               // ====================================================

               ImGui::TableSetupColumn(
                    "Event",
                    ImGuiTableColumnFlags_WidthFixed,
                    220.0f
               );

               ImGui::TableSetupColumn(
                    "Timeline",
                    ImGuiTableColumnFlags_WidthStretch
               );


               ImGui::TableHeadersRow();


               // ====================================================
               // Events
               // ====================================================

               for (std::size_t i = 0;
                    i < events.size();
                    ++i)
               {
                    const auto& event = events[i];


                    ImGui::PushID(
                         static_cast<int>(i)
                    );


                    ImGui::TableNextRow(
                         ImGuiTableRowFlags_None,
                         rowHeight
                    );


                    // =================================================
                    // Column 0
                    // Event Name
                    // =================================================

                    ImGui::TableSetColumnIndex(0);


                    const float indent =
                         static_cast<float>(
                              event.depth
                              ) * 14.0f;


                    ImGui::Indent(indent);


                    ImGui::TextUnformatted(
                         event.name.c_str()
                    );


                    ImGui::Unindent(indent);


                    // =================================================
                    // Column 1
                    // Timeline
                    // =================================================

                    ImGui::TableSetColumnIndex(1);


                    const ImVec2 timelinePosition =
                         ImGui::GetCursorScreenPos();


                    const float timelineWidth =
                         ImGui::GetContentRegionAvail().x;


                    // 防止窗口太窄
                    if (timelineWidth > 1.0f)
                    {
                         DrawTimelineEvent(
                              event,
                              timelinePosition,
                              timelineWidth,
                              rowHeight,
                              frameTime
                         );
                    }


                    // 占据这一行的空间
                    ImGui::Dummy(
                         ImVec2(
                              timelineWidth,
                              rowHeight
                         )
                    );


                    ImGui::PopID();
               }


               ImGui::EndTable();
          }
     }
     void DrawTimelineEvent(
          const ProfileEvent& event,
          const ImVec2& timelinePosition,
          float timelineWidth,
          float rowHeight,
          float frameTime)
     {
          ImDrawList* drawList =
               ImGui::GetWindowDrawList();


          // ========================================================
          // Calculate Position
          // ========================================================

          const float startRatio =
               event.startMs /
               frameTime;


          const float durationRatio =
               event.durationMs /
               frameTime;


          const float barStartX =
               timelinePosition.x +
               startRatio *
               timelineWidth;


          const float barWidth =
               std::max(
                    1.0f,
                    durationRatio *
                    timelineWidth
               );


          // ========================================================
          // Bar Rectangle
          // ========================================================

          const ImVec2 barMin(
               barStartX,
               timelinePosition.y + 3.0f
          );


          const ImVec2 barMax(
               barStartX + barWidth,
               timelinePosition.y +
               rowHeight -
               3.0f
          );


          // ========================================================
          // Draw
          // ========================================================

          drawList->AddRectFilled(
               barMin,
               barMax,
               GetDepthColor(event.depth),
               2.0f
          );


          // ========================================================
          // Hover
          // ========================================================

          const ImVec2 mouse =
               ImGui::GetMousePos();


          const bool hovered =
               mouse.x >= barMin.x &&
               mouse.x <= barMax.x &&
               mouse.y >= barMin.y &&
               mouse.y <= barMax.y;


          if (hovered)
          {
               ImGui::BeginTooltip();

               ImGui::Text(
                    "%s",
                    event.name.c_str()
               );

               ImGui::Separator();

               ImGui::Text(
                    "Start: %.3f ms",
                    event.startMs
               );

               ImGui::Text(
                    "Duration: %.3f ms",
                    event.durationMs
               );

               ImGui::Text(
                    "End: %.3f ms",
                    event.startMs +
                    event.durationMs
               );

               ImGui::Text(
                    "Depth: %u",
                    event.depth
               );

               ImGui::EndTooltip();
          }
     }

     // ========================================================
     // Event Table
     // ========================================================

     void DrawEventTable(
          const std::vector<ProfileEvent>& frame)
     {
          ImGui::Text("Events");


          std::vector<ProfileEvent> events =
               frame;


          SortEvents(events);


          if (ImGui::BeginTable(
               "ProfilerEventTable",
               4,
               ImGuiTableFlags_Borders |
               ImGuiTableFlags_RowBg |
               ImGuiTableFlags_Resizable |
               ImGuiTableFlags_ScrollY,
               ImVec2(0.0f, 250.0f)))
          {
               ImGui::TableSetupColumn(
                    "Section",
                    ImGuiTableColumnFlags_WidthStretch
               );

               ImGui::TableSetupColumn(
                    "Start",
                    ImGuiTableColumnFlags_WidthFixed,
                    90.0f
               );

               ImGui::TableSetupColumn(
                    "Duration",
                    ImGuiTableColumnFlags_WidthFixed,
                    100.0f
               );

               ImGui::TableSetupColumn(
                    "Depth",
                    ImGuiTableColumnFlags_WidthFixed,
                    60.0f
               );


               ImGui::TableHeadersRow();


               for (const auto& event : events)
               {
                    ImGui::TableNextRow();


                    // ============================================
                    // Name
                    // ============================================

                    ImGui::TableSetColumnIndex(0);


                    const float indent =
                         static_cast<float>(
                              event.depth
                              ) * 15.0f;


                    ImGui::Indent(indent);

                    ImGui::TextUnformatted(
                         event.name.c_str()
                    );

                    ImGui::Unindent(indent);


                    // ============================================
                    // Start
                    // ============================================

                    ImGui::TableSetColumnIndex(1);

                    ImGui::Text(
                         "%.3f ms",
                         event.startMs
                    );


                    // ============================================
                    // Duration
                    // ============================================

                    ImGui::TableSetColumnIndex(2);

                    ImGui::Text(
                         "%.3f ms",
                         event.durationMs
                    );


                    // ============================================
                    // Depth
                    // ============================================

                    ImGui::TableSetColumnIndex(3);

                    ImGui::Text(
                         "%u",
                         event.depth
                    );
               }


               ImGui::EndTable();
          }
     }


     // ========================================================
     // Utility
     // ========================================================

     static void SortEvents(
          std::vector<ProfileEvent>& events)
     {
          std::sort(
               events.begin(),
               events.end(),
               [](const ProfileEvent& a,
                    const ProfileEvent& b)
               {
                    if (a.startMs ==
                         b.startMs)
                    {
                         return a.depth <
                              b.depth;
                    }

                    return a.startMs <
                         b.startMs;
               }
          );
     }


     static float CalculateFrameTime(
          const std::vector<ProfileEvent>& frame)
     {
          float endTime = 0.0f;


          for (const auto& event : frame)
          {
               endTime =
                    std::max(
                         endTime,
                         event.startMs +
                         event.durationMs
                    );
          }


          return endTime;
     }


     static void DrawReferenceLine(
          ImDrawList* drawList,
          const ImVec2& graphPosition,
          float graphWidth,
          float graphHeight,
          float maxFrameTime,
          float referenceTime,
          const char* label)
     {
          if (maxFrameTime <= 0.0f)
               return;


          const float ratio =
               referenceTime /
               maxFrameTime;


          if (ratio > 1.0f)
               return;


          const float y =
               graphPosition.y +
               graphHeight -
               ratio *
               graphHeight;


          drawList->AddLine(
               ImVec2(
                    graphPosition.x,
                    y
               ),
               ImVec2(
                    graphPosition.x +
                    graphWidth,
                    y
               ),
               ImGui::GetColorU32(
                    ImGuiCol_TextDisabled
               )
          );


          drawList->AddText(
               ImVec2(
                    graphPosition.x + 4.0f,
                    y + 2.0f
               ),
               ImGui::GetColorU32(
                    ImGuiCol_TextDisabled
               ),
               label
          );
     }


     static ImU32 GetDepthColor(
          uint32_t depth)
     {
          // 不硬编码颜色。
          // 从当前 ImGui Theme 中选择几个颜色，
          // 这样 Dark / Light Theme 都能工作。

          switch (depth % 4)
          {
          case 0:
               return ImGui::GetColorU32(
                    ImGuiCol_ButtonActive
               );

          case 1:
               return ImGui::GetColorU32(
                    ImGuiCol_PlotHistogram
               );

          case 2:
               return ImGui::GetColorU32(
                    ImGuiCol_HeaderActive
               );

          default:
               return ImGui::GetColorU32(
                    ImGuiCol_SliderGrabActive
               );
          }
     }


private:

     // -1 表示还没有选择
     int m_SelectedFrameIndex = -1;

     // true:
     // 每帧自动选择最新 Frame
     //
     // false:
     // 用户正在查看历史 Frame
     bool m_FollowLatestFrame = true;
};