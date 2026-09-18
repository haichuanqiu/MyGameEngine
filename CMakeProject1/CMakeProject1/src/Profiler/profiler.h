#pragma once

#include <chrono>
#include <cstdint>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>


// ============================================================
// Profile Event
// ============================================================

struct ProfileEvent
{
     std::string name;

     // 相对于这一帧开始时间
     float startMs = 0.0f;

     // 这个 scope 自己执行了多久
     float durationMs = 0.0f;

     // Scope 嵌套深度
     uint32_t depth = 0;
};


// ============================================================
// Profiler Manager
// ============================================================

class ProfilerManager
{
public:

     using Clock = std::chrono::steady_clock;
     using TimePoint = Clock::time_point;


     // --------------------------------------------------------
     // Singleton
     // --------------------------------------------------------

     static ProfilerManager& Instance()
     {
          static ProfilerManager instance;
          return instance;
     }


     // --------------------------------------------------------
     // Recording
     // --------------------------------------------------------

     void StartRecording()
     {
          m_IsRecording = true;
     }

     void StopRecording()
     {
          m_IsRecording = false;
     }

     bool IsRecording() const
     {
          return m_IsRecording;
     }


     // --------------------------------------------------------
     // Frame
     // --------------------------------------------------------

     void NewFrame()
     {
          if (!m_IsRecording)
               return;

          // 这一帧的时间原点
          m_FrameStartTime = Clock::now();

          // 新建一帧
          m_Frames.emplace_back();

          // 防止无限保存
          if (m_Frames.size() > m_MaxStoredFrames)
          {
               m_Frames.erase(m_Frames.begin());
          }

          // 正常来说一帧开始时应该没有任何 scope
          m_CurrentDepth = 0;
     }


     // --------------------------------------------------------
     // Scope
     // --------------------------------------------------------

     uint32_t BeginScope()
     {
          if (!m_IsRecording)
               return 0;

          // 当前 depth 属于这个 scope
          const uint32_t depth = m_CurrentDepth;

          // 接下来进入更深一层
          ++m_CurrentDepth;

          return depth;
     }


     void EndScope()
     {
          if (!m_IsRecording)
               return;

          if (m_CurrentDepth > 0)
          {
               --m_CurrentDepth;
          }
     }


     // --------------------------------------------------------
     // Time
     // --------------------------------------------------------

     float GetTimeFromFrameStartMs(TimePoint time) const
     {
          return std::chrono::duration<float, std::milli>(
               time - m_FrameStartTime
          ).count();
     }


     // --------------------------------------------------------
     // Submit
     // --------------------------------------------------------

     void AddEvent(ProfileEvent event)
     {
          if (!m_IsRecording)
               return;

          if (m_Frames.empty())
               return;

          m_Frames.back().push_back(
               std::move(event)
          );
     }


     // --------------------------------------------------------
     // Data Access
     // --------------------------------------------------------

     const std::vector<ProfileEvent>& GetCurrentFrame() const
     {
          static const std::vector<ProfileEvent> emptyFrame;

          if (m_Frames.empty())
               return emptyFrame;

          return m_Frames.back();
     }


     const std::vector<std::vector<ProfileEvent>>&
          GetFrames() const
     {
          return m_Frames;
     }


     void Clear()
     {
          m_Frames.clear();
          m_CurrentDepth = 0;
     }


private:

     ProfilerManager() = default;
     ~ProfilerManager() = default;

     ProfilerManager(const ProfilerManager&) = delete;
     ProfilerManager& operator=(const ProfilerManager&) = delete;

     ProfilerManager(ProfilerManager&&) = delete;
     ProfilerManager& operator=(ProfilerManager&&) = delete;


private:

     bool m_IsRecording = false;

     std::size_t m_MaxStoredFrames = 120;

     uint32_t m_CurrentDepth = 0;

     TimePoint m_FrameStartTime{};

     std::vector<std::vector<ProfileEvent>> m_Frames;
};


// ============================================================
// Profile Timer
// ============================================================

class ProfileTimer
{
public:

     using Clock = std::chrono::steady_clock;


     explicit ProfileTimer(std::string name)
          : m_Name(std::move(name))
     {
          auto& profiler =
               ProfilerManager::Instance();

          // Profiler 没开的话，这个 Timer 什么都不做
          if (!profiler.IsRecording())
               return;

          m_IsActive = true;

          m_Depth = profiler.BeginScope();

          m_Start = Clock::now();
     }


     ~ProfileTimer()
     {
          if (!m_IsActive)
               return;

          const auto end = Clock::now();

          auto& profiler =
               ProfilerManager::Instance();


          // ----------------------------
          // Start Time
          // ----------------------------

          const float startMs =
               profiler.GetTimeFromFrameStartMs(
                    m_Start
               );


          // ----------------------------
          // Duration
          // ----------------------------

          const float durationMs =
               std::chrono::duration<float, std::milli>(
                    end - m_Start
               ).count();


          // ----------------------------
          // Submit Event
          // ----------------------------

          profiler.AddEvent(
               ProfileEvent{
                   m_Name,
                   startMs,
                   durationMs,
                   m_Depth
               }
          );


          // 离开当前 Scope
          profiler.EndScope();
     }


     ProfileTimer(const ProfileTimer&) = delete;
     ProfileTimer& operator=(const ProfileTimer&) = delete;


private:

     std::string m_Name;

     Clock::time_point m_Start{};

     uint32_t m_Depth = 0;

     bool m_IsActive = false;
};


// ============================================================
// Macros
// ============================================================

#define PROFILE_CONCAT_INTERNAL(x, y) x##y
#define PROFILE_CONCAT(x, y) PROFILE_CONCAT_INTERNAL(x, y)

#define ENGINE_PROFILE_SCOPE(name) \
    ProfileTimer PROFILE_CONCAT(profileTimer_, __LINE__)(name)

#define ENGINE_PROFILE_FUNCTION() \
    ENGINE_PROFILE_SCOPE(__FUNCTION__)