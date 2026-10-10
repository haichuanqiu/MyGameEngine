#pragma once

#include <chrono>
#include <cstdint>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <algorithm>

// ============================================================
// Profile Event
// ============================================================

struct ProfileEvent
{
     std::string name;

     float startMs = 0.0f;
     float durationMs = 0.0f;

     uint32_t depth = 0;

     // 新增：事件来自哪个 Thread
     std::thread::id threadId{};

     // 新增：事件属于哪一帧
     uint64_t frameId = 0;
};


// ============================================================
// Profile Frame
// ============================================================

struct ProfileFrame
{
     uint64_t frameId = 0;

     std::vector<ProfileEvent> events;
};


// ============================================================
// Profiler Manager
// ============================================================

class ProfilerManager
{
public:

     using Clock = std::chrono::steady_clock;
     using TimePoint = Clock::time_point;

     struct ScopeContext
     {
          TimePoint frameStart{};
          uint64_t frameId = 0;
          uint32_t depth = 0;
          bool active = false;
     };


     static ProfilerManager& Instance()
     {
          static ProfilerManager instance;
          return instance;
     }


     // ========================================================
     // Recording
     // ========================================================

     void StartRecording()
     {
          std::lock_guard<std::mutex> lock(m_Mutex);

          m_IsRecording = true;
     }


     void StopRecording()
     {
          std::lock_guard<std::mutex> lock(m_Mutex);

          m_IsRecording = false;
     }


     bool IsRecording() const
     {
          return m_IsRecording.load();
     }


     // ========================================================
     // Frame
     //
     // 只允许 Game Thread 调用。
     // ========================================================

     void NewFrame()
     {
          std::lock_guard<std::mutex> lock(m_Mutex);

          if (!m_IsRecording)
               return;

          ++m_CurrentFrameId;

          m_FrameStartTime = Clock::now();

          ProfileFrame frame;
          frame.frameId = m_CurrentFrameId;

          m_Frames.push_back(std::move(frame));

          if (m_Frames.size() > m_MaxStoredFrames)
          {
               m_Frames.erase(m_Frames.begin());
          }
     }


     // ========================================================
     // Begin Scope
     //
     // 每个 Thread 都有自己的 depth。
     // ========================================================

     ScopeContext BeginScope()
     {
          ScopeContext context;

          std::lock_guard<std::mutex> lock(m_Mutex);

          if (!m_IsRecording || m_Frames.empty())
               return context;

          context.active = true;
          context.frameId = m_CurrentFrameId;
          context.frameStart = m_FrameStartTime;

          // 当前线程的独立嵌套深度
          context.depth = s_ThreadDepth++;

          return context;
     }


     // ========================================================
     // End Scope
     // ========================================================

     void EndScope()
     {
          if (s_ThreadDepth > 0)
          {
               --s_ThreadDepth;
          }
     }


     // ========================================================
     // Submit Event
     //
     // 可以被多个 Worker 同时调用。
     // ========================================================

     void AddEvent(ProfileEvent event)
     {
          std::lock_guard<std::mutex> lock(m_Mutex);

          // 根据 frameId 找对应的帧，
          // 而不是直接使用 m_Frames.back()。
          auto it = std::find_if(
               m_Frames.begin(),
               m_Frames.end(),
               [&](const ProfileFrame& frame)
               {
                    return frame.frameId == event.frameId;
               }
          );

          if (it == m_Frames.end())
          {
               // 事件所属帧已经被删除。
               return;
          }

          it->events.push_back(std::move(event));
     }


     // ========================================================
     // Data Access
     //
     // 返回 Snapshot，避免 UI 读取时 Worker 正在写。
     // ========================================================

     std::vector<ProfileEvent> GetCurrentFrame() const
     {
          std::lock_guard<std::mutex> lock(m_Mutex);

          if (m_Frames.empty())
               return {};

          return m_Frames.back().events;
     }


     std::vector<ProfileFrame> GetFrames() const
     {
          std::lock_guard<std::mutex> lock(m_Mutex);

          return m_Frames;
     }


     void Clear()
     {
          std::lock_guard<std::mutex> lock(m_Mutex);

          m_Frames.clear();
     }


private:

     ProfilerManager() = default;
     ~ProfilerManager() = default;

     ProfilerManager(const ProfilerManager&) = delete;
     ProfilerManager& operator=(const ProfilerManager&) = delete;


private:

     mutable std::mutex m_Mutex;

     std::atomic<bool> m_IsRecording{ false };

     std::size_t m_MaxStoredFrames = 120;

     uint64_t m_CurrentFrameId = 0;

     TimePoint m_FrameStartTime{};

     std::vector<ProfileFrame> m_Frames;

     // 每个线程有独立的 Scope 嵌套深度。
     inline static thread_local uint32_t s_ThreadDepth = 0;
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
          auto& profiler = ProfilerManager::Instance();

          m_Context = profiler.BeginScope();

          if (!m_Context.active)
               return;

          m_Start = Clock::now();
     }


     ~ProfileTimer()
     {
          if (!m_Context.active)
               return;

          const auto end = Clock::now();

          const float startMs =
               std::chrono::duration<float, std::milli>(
                    m_Start - m_Context.frameStart
               ).count();

          const float durationMs =
               std::chrono::duration<float, std::milli>(
                    end - m_Start
               ).count();


          ProfileEvent event;

          event.name = m_Name;
          event.startMs = startMs;
          event.durationMs = durationMs;
          event.depth = m_Context.depth;

          event.threadId = std::this_thread::get_id();
          event.frameId = m_Context.frameId;


          auto& profiler = ProfilerManager::Instance();

          profiler.AddEvent(std::move(event));

          profiler.EndScope();
     }


     ProfileTimer(const ProfileTimer&) = delete;
     ProfileTimer& operator=(const ProfileTimer&) = delete;


private:

     std::string m_Name;

     Clock::time_point m_Start{};

     ProfilerManager::ScopeContext m_Context;
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