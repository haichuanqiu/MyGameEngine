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


     void Submit(
          std::function<void()> function,
          JobGroup* group = nullptr)
     {
          Job job;

          job.function = std::move(function);
          job.group = group;


          if (group != nullptr)
          {
               group->AddJob();
          }


          jobQueue.Push(std::move(job));
     }


     void Shutdown()
     {
          // 告诉 Queue：
          //
          // 不会再有 Job 了
          //
          jobQueue.Shutdown();


          // 等所有 Worker Thread 退出
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
