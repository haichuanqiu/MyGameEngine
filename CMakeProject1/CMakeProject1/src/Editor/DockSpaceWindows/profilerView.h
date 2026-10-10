#pragma once

#include "imgui.h"
#include "Profiler/profiler.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <thread>
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

          // 多线程 Profiler 返回的是安全快照。
          auto frames = profiler.GetFrames();

          if (frames.empty())
          {
               ImGui::TextDisabled("No profiling data.");
               ImGui::End();
               return;
          }

          ValidateSelectedFrame(frames);

          DrawFrameHistory(frames);

          ImGui::Separator();

          const ProfileFrame& selectedFrame =
               frames[m_SelectedFrameIndex];

          const auto& events = selectedFrame.events;

          const float profileSpan =
               CalculateFrameTime(events);

          ImGui::Text(
               "Frame ID: %llu",
               static_cast<unsigned long long>(
                    selectedFrame.frameId
                    )
          );

          ImGui::SameLine();

          ImGui::Text(
               "| Profile Span: %.3f ms",
               profileSpan
          );

          ImGui::SameLine();

          ImGui::Text(
               "| Events: %zu",
               events.size()
          );

          ImGui::Separator();

          DrawTimeline(events);

          ImGui::Separator();

          DrawEventTable(events);

          ImGui::End();
     }


private:

     // ============================================================
     // Toolbar
     // ============================================================

     void DrawToolbar(ProfilerManager& profiler)
     {
          if (profiler.IsRecording())
          {
               ImGui::Text("Recording: ON");
               ImGui::SameLine();

               if (ImGui::Button("Stop"))
               {
                    profiler.StopRecording();
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
     }


     // ============================================================
     // Frame Selection
     // ============================================================

     void ValidateSelectedFrame(
          const std::vector<ProfileFrame>& frames)
     {
          if (frames.empty())
          {
               m_SelectedFrameIndex = -1;
               return;
          }

          if (m_FollowLatestFrame)
          {
               m_SelectedFrameIndex =
                    static_cast<int>(frames.size()) - 1;

               return;
          }

          m_SelectedFrameIndex = std::clamp(
               m_SelectedFrameIndex,
               0,
               static_cast<int>(frames.size()) - 1
          );
     }


     // ============================================================
     // Frame History
     // ============================================================

     void DrawFrameHistory(
          const std::vector<ProfileFrame>& frames)
     {
          ImGui::Text(
               "Frame History (%zu)",
               frames.size()
          );

          std::vector<float> frameTimes;

          frameTimes.reserve(frames.size());

          float maxTime = 16.67f;

          for (const auto& frame : frames)
          {
               const float time =
                    CalculateFrameTime(frame.events);

               frameTimes.push_back(time);

               maxTime = std::max(maxTime, time);
          }

          maxTime *= 1.1f;

          const float width =
               ImGui::GetContentRegionAvail().x;

          const float height = 100.0f;

          if (width <= 0.0f)
               return;

          const ImVec2 origin =
               ImGui::GetCursorScreenPos();

          ImDrawList* drawList =
               ImGui::GetWindowDrawList();

          const float barWidth =
               width / static_cast<float>(frames.size());

          // 16.67ms reference line
          const float referenceY =
               origin.y + height -
               (16.67f / maxTime) * height;

          drawList->AddLine(
               ImVec2(origin.x, referenceY),
               ImVec2(origin.x + width, referenceY),
               ImGui::GetColorU32(ImGuiCol_TextDisabled)
          );

          for (size_t i = 0; i < frames.size(); ++i)
          {
               const float ratio =
                    std::clamp(
                         frameTimes[i] / maxTime,
                         0.0f,
                         1.0f
                    );

               const float x0 =
                    origin.x + static_cast<float>(i) * barWidth;

               const float x1 =
                    x0 + std::max(1.0f, barWidth - 1.0f);

               const float y0 =
                    origin.y + height - ratio * height;

               const float y1 =
                    origin.y + height;

               const ImU32 color =
                    static_cast<int>(i) == m_SelectedFrameIndex
                    ? ImGui::GetColorU32(ImGuiCol_ButtonActive)
                    : ImGui::GetColorU32(ImGuiCol_PlotHistogram);

               drawList->AddRectFilled(
                    ImVec2(x0, y0),
                    ImVec2(x1, y1),
                    color
               );
          }

          ImGui::InvisibleButton(
               "##FrameHistory",
               ImVec2(width, height)
          );

          if (ImGui::IsItemHovered())
          {
               const float mouseX =
                    ImGui::GetMousePos().x - origin.x;

               const int index = std::clamp(
                    static_cast<int>(mouseX / barWidth),
                    0,
                    static_cast<int>(frames.size()) - 1
               );

               ImGui::BeginTooltip();

               ImGui::Text(
                    "Frame ID: %llu",
                    static_cast<unsigned long long>(
                         frames[index].frameId
                         )
               );

               ImGui::Text(
                    "Profile Span: %.3f ms",
                    frameTimes[index]
               );

               ImGui::EndTooltip();

               if (ImGui::IsMouseClicked(
                    ImGuiMouseButton_Left))
               {
                    m_SelectedFrameIndex = index;
                    m_FollowLatestFrame = false;
               }
          }
     }


     // ============================================================
     // Timeline
     //
     // 每个 Thread 独立显示
     // ============================================================

     void DrawTimeline(
          const std::vector<ProfileEvent>& frame)
     {
          ImGui::Text("Multithread Timeline");

          if (frame.empty())
          {
               ImGui::TextDisabled("No events.");
               return;
          }

          // 找出所有出现过的线程。
          std::vector<std::thread::id> threadIds;

          for (const auto& event : frame)
          {
               if (std::find(
                    threadIds.begin(),
                    threadIds.end(),
                    event.threadId
               ) == threadIds.end())
               {
                    threadIds.push_back(event.threadId);
               }
          }

          // 同一线程内部按开始时间排序。
          std::vector<ProfileEvent> events = frame;

          std::sort(
               events.begin(),
               events.end(),
               [](const ProfileEvent& a,
                    const ProfileEvent& b)
               {
                    if (a.startMs == b.startMs)
                         return a.depth < b.depth;

                    return a.startMs < b.startMs;
               }
          );

          const float frameTime =
               CalculateFrameTime(events);

          if (frameTime <= 0.0f)
               return;

          ImGui::Text(
               "Threads: %zu",
               threadIds.size()
          );

          constexpr float rowHeight = 22.0f;

          if (ImGui::BeginTable(
               "ProfilerTimeline",
               2,
               ImGuiTableFlags_Borders |
               ImGuiTableFlags_RowBg |
               ImGuiTableFlags_Resizable |
               ImGuiTableFlags_ScrollY,
               ImVec2(0.0f, 350.0f)))
          {
               ImGui::TableSetupColumn(
                    "Thread / Event",
                    ImGuiTableColumnFlags_WidthFixed,
                    220.0f
               );

               ImGui::TableSetupColumn(
                    "Timeline",
                    ImGuiTableColumnFlags_WidthStretch
               );

               ImGui::TableHeadersRow();

               for (size_t threadIndex = 0;
                    threadIndex < threadIds.size();
                    ++threadIndex)
               {
                    const auto threadId =
                         threadIds[threadIndex];

                    // Thread Header
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);

                    ImGui::Text(
                         "Thread %zu",
                         threadIndex
                    );

                    ImGui::SameLine();

                    ImGui::TextDisabled(
                         "(%zu)",
                         std::hash<std::thread::id>{}(threadId)
                    );

                    // Events for this Thread
                    for (size_t eventIndex = 0;
                         eventIndex < events.size();
                         ++eventIndex)
                    {
                         const auto& event =
                              events[eventIndex];

                         if (event.threadId != threadId)
                              continue;

                         ImGui::PushID(
                              static_cast<int>(eventIndex)
                         );

                         ImGui::TableNextRow(
                              ImGuiTableRowFlags_None,
                              rowHeight
                         );

                         // Event Name
                         ImGui::TableSetColumnIndex(0);

                         const float indent =
                              static_cast<float>(event.depth)
                              * 12.0f;

                         ImGui::Indent(indent);

                         ImGui::TextUnformatted(
                              event.name.c_str()
                         );

                         ImGui::Unindent(indent);

                         // Timeline Bar
                         ImGui::TableSetColumnIndex(1);

                         const ImVec2 position =
                              ImGui::GetCursorScreenPos();

                         const float width =
                              ImGui::GetContentRegionAvail().x;

                         if (width > 1.0f)
                         {
                              DrawTimelineEvent(
                                   event,
                                   position,
                                   width,
                                   rowHeight,
                                   frameTime
                              );
                         }

                         ImGui::Dummy(
                              ImVec2(
                                   std::max(1.0f, width),
                                   rowHeight
                              )
                         );

                         ImGui::PopID();
                    }
               }

               ImGui::EndTable();
          }
     }


     // ============================================================
     // Draw Timeline Event
     // ============================================================

     void DrawTimelineEvent(
          const ProfileEvent& event,
          const ImVec2& position,
          float width,
          float rowHeight,
          float frameTime)
     {
          if (frameTime <= 0.0f)
               return;

          ImDrawList* drawList =
               ImGui::GetWindowDrawList();

          const float startRatio =
               event.startMs / frameTime;

          const float durationRatio =
               event.durationMs / frameTime;

          const float x0 =
               position.x + startRatio * width;

          const float x1 =
               x0 + std::max(
                    1.0f,
                    durationRatio * width
               );

          const ImVec2 min(
               x0,
               position.y + 3.0f
          );

          const ImVec2 max(
               x1,
               position.y + rowHeight - 3.0f
          );

          drawList->AddRectFilled(
               min,
               max,
               GetDepthColor(event.depth),
               2.0f
          );

          const ImVec2 mouse =
               ImGui::GetMousePos();

          if (mouse.x >= min.x &&
               mouse.x <= max.x &&
               mouse.y >= min.y &&
               mouse.y <= max.y)
          {
               ImGui::BeginTooltip();

               ImGui::TextUnformatted(
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
                    "Depth: %u",
                    event.depth
               );

               ImGui::Text(
                    "Thread: %zu",
                    std::hash<std::thread::id>{}(
                         event.threadId
                         )
               );

               ImGui::EndTooltip();
          }
     }


     // ============================================================
     // Event Table
     // ============================================================

     void DrawEventTable(
          const std::vector<ProfileEvent>& frame)
     {
          ImGui::Text("Events");

          std::vector<ProfileEvent> events = frame;

          std::sort(
               events.begin(),
               events.end(),
               [](const ProfileEvent& a,
                    const ProfileEvent& b)
               {
                    return a.startMs < b.startMs;
               }
          );

          if (ImGui::BeginTable(
               "ProfilerEventTable",
               5,
               ImGuiTableFlags_Borders |
               ImGuiTableFlags_RowBg |
               ImGuiTableFlags_Resizable |
               ImGuiTableFlags_ScrollY,
               ImVec2(0.0f, 250.0f)))
          {
               ImGui::TableSetupColumn("Event");
               ImGui::TableSetupColumn("Thread");
               ImGui::TableSetupColumn("Start");
               ImGui::TableSetupColumn("Duration");
               ImGui::TableSetupColumn("Depth");

               ImGui::TableHeadersRow();

               for (const auto& event : events)
               {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(event.name.c_str());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text(
                         "%zu",
                         std::hash<std::thread::id>{}(
                              event.threadId
                              )
                    );

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.3f ms", event.startMs);

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.3f ms", event.durationMs);

                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%u", event.depth);
               }

               ImGui::EndTable();
          }

          // ============================================================
          // Copy Events Table
          // ============================================================

          if (ImGui::Button("Copy Table"))
          {
               std::string clipboard;

               clipboard +=
                    "Event\tThread\tStart\tDuration\tDepth\n";

               char buffer[512];

               for (const auto& event : events)
               {
                    std::snprintf(
                         buffer,
                         sizeof(buffer),
                         "%s\t%zu\t%.3f ms\t%.3f ms\t%u\n",
                         event.name.c_str(),
                         std::hash<std::thread::id>{}(
                              event.threadId
                              ),
                         event.startMs,
                         event.durationMs,
                         event.depth
                    );

                    clipboard += buffer;
               }

               ImGui::SetClipboardText(clipboard.c_str());
          }
     }



     // ============================================================
     // Utility
     // ============================================================

     static float CalculateFrameTime(
          const std::vector<ProfileEvent>& events)
     {
          float endTime = 0.0f;

          for (const auto& event : events)
          {
               endTime = std::max(
                    endTime,
                    event.startMs + event.durationMs
               );
          }

          return endTime;
     }


     static ImU32 GetDepthColor(uint32_t depth)
     {
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

     int m_SelectedFrameIndex = -1;

     bool m_FollowLatestFrame = true;
};