#pragma once
#include "Profiler/profiler.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <iostream>
class JobGroup
{
public:
     void AddJob()
     {
          remaining.fetch_add(1);
     }

     void JobFinished()
     {
          int oldValue = remaining.fetch_sub(1);
          if (oldValue == 1)
          {

               cv.notify_all();
          }
     }

     void Wait()
     {
          ENGINE_PROFILE_SCOPE("JobGroup::Wait");
          std::unique_lock<std::mutex> lock(mutex);

          cv.wait(lock, [this]()
               {
                    return remaining.load() == 0;
               });
     }

private:
     std::atomic<int> remaining{ 0 };

     std::mutex mutex;
     std::condition_variable cv;
};
