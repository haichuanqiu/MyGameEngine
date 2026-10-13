#pragma once

#include "Worker.h"
#include "JobGroup.h"

#include <vector>
#include <memory>

class ThreadPool
{
public:

     void Start(int workerCount)
     {
          for (int i = 0; i < workerCount; ++i)
          {
               auto worker = std::make_unique<Worker>();

               worker->Start(&jobQueue);

               workers.push_back(std::move(worker));
          }
     }


     bool Submit(std::unique_ptr<Job> job)
     {
          if (!job)
          {
               return false;
          }

          JobGroup* group = job->group;

          if (group != nullptr)
          {
               group->AddJob();
          }

          if (!jobQueue.Push(std::move(job)))
          {
               // Push 失败时恢复 JobGroup 计数
               if (group != nullptr)
               {
                    group->JobFinished();
               }

               return false;
          }

          return true;
     }


     void Shutdown()
     {
          // 不再接受新 Job
          jobQueue.Shutdown();

          // 等待所有 Worker 退出
          for (auto& worker : workers)
          {
               worker->Join();
          }

          workers.clear();
     }


private:

     JobQueue jobQueue;

     std::vector<std::unique_ptr<Worker>> workers;
};