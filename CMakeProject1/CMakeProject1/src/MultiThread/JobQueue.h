#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "Job.h"

class JobQueue
{
public:

     bool Push(std::unique_ptr<Job> job)
     {
          {
               std::lock_guard<std::mutex> lock(mutex);

               if (shuttingDown)
               {
                    return false;
               }

               jobs.push(std::move(job));
          }

          cv.notify_one();

          return true;
     }


     bool Pop(std::unique_ptr<Job>& outJob)
     {
          std::unique_lock<std::mutex> lock(mutex);

          cv.wait(lock, [this]()
               {
                    return !jobs.empty() || shuttingDown;
               });

          if (shuttingDown && jobs.empty())
          {
               return false;
          }

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

          cv.notify_all();
     }


private:

     std::queue<std::unique_ptr<Job>> jobs;

     std::mutex mutex;

     std::condition_variable cv;

     bool shuttingDown = false;
};