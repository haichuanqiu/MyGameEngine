#pragma once

#include "Job.h"
#include "Profiler/profiler.h"

#include <queue>
#include <mutex>
#include <condition_variable>

class JobQueue
{
public:

     void Push(Job job)
     {

               std::lock_guard<std::mutex> lock(mutex);

               jobs.push(std::move(job));
          

          // 有新工作
          // 叫醒一个 Worker
          cv.notify_one();
     }


     bool Pop(Job& outJob)
     {
          std::unique_lock<std::mutex> lock(mutex);

          // 如果：
          //
          // jobs.empty() == true
          //
          // Worker 就睡在这里
          //
          cv.wait(lock, [this]()
               {
                    return !jobs.empty() || shuttingDown;
               });


          // Engine 正在关闭
          if (shuttingDown && jobs.empty())
          {
               return false;
          }


          // 拿走一个 Job
          outJob = std::move(jobs.front());
          jobs.pop();

          return true;
     }


     void Shutdown()
     {
          {
               std::lock_guard<std::mutex> lock(mutex);

               shuttingDown = true;
          }

          // 把所有正在睡觉的 Worker 叫醒
          cv.notify_all();
     }


private:

     std::queue<Job> jobs;

     std::mutex mutex;

     std::condition_variable cv;

     bool shuttingDown = false;
};
