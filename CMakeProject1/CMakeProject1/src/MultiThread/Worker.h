#pragma once

#include "JobQueue.h"

#include <thread>

class Worker
{
public:

     void Start(JobQueue* queue)
     {
          jobQueue = queue;

          thread = std::thread(
               &Worker::WorkerLoop,
               this
          );
     }


     void Join()
     {
          if (thread.joinable())
          {
               thread.join();
          }
     }


private:

     void WorkerLoop()
     {
          while (true)
          {
               Job job;

               // 没 Job 时会睡在 Pop() 里面
               if (!jobQueue->Pop(job))
               {
                    // Queue Shutdown
                    break;
               }

               // 真正执行任务
               job.Execute();
          }
     }


private:

     JobQueue* jobQueue = nullptr;

     std::thread thread;
};