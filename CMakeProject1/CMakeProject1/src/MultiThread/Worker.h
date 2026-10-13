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

               std::unique_ptr<Job> job;

               while (jobQueue->Pop(job))
               {
                    job->Execute();

                    job.reset();
               }

               // 真正执行任务
               job->Execute();
          }
     }


private:

     JobQueue* jobQueue = nullptr;

     std::thread thread;
};