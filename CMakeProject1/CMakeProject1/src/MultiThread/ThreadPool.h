#pragma once

#include "Worker.h"
#include "JobGroup.h"
#include "internalQueuedJob.h"
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


     void Submit(std::unique_ptr<Job> job)
     {
          if (!job)
          {
               return;
          }

          // 将 Job 的所有权转移给 shared_ptr
          std::shared_ptr<Job> ownedJob(std::move(job));

          // 真正创建 QueuedJob 对象
          auto queued = std::make_unique<QueuedJob>();

          // 保存执行函数，并持有 Job 对象
          queued->function = [ownedJob]()
               {
                    ownedJob->ToExecute();
               };
          queued->group = ownedJob->group;

          // 提交前增加 JobGroup 计数
          if (queued->group)
          {
               queued->group->AddJob();
          }

          jobQueue.Push(std::move(queued));
     }
     void Submit(
          std::unique_ptr<JobParallelFor> job,
          int length,
          int startIndex)
     {
          const int batchSize = job->batchSize;
          if (!job || length <= 0)
          {
               std::cerr << "submited invalid job or length " << std::endl;
               return;
          }

          if (batchSize <= 0)
          {
               std::cerr << "submited jon with invalid batch size " << std::endl;
               return;
          }
          std::shared_ptr<JobParallelFor> ownedJob(std::move(job));
         

          // 整个任务要处理的索引范围是：
          // [startIndex, startIndex + length)
          const int endIndex = startIndex + length;

          for (int batchStart = startIndex;
               batchStart < endIndex;
               batchStart += batchSize)
          {
               const int batchEnd = std::min(
                    batchStart + batchSize,
                    endIndex
               );

               auto queued = std::make_unique<QueuedJob>();

               queued->function =
                    [ownedJob, batchStart, batchEnd]()
                    {
                         for (int i = batchStart; i < batchEnd; ++i)
                         {
                              ownedJob->ToExecute(i);
                         }
                    };

               queued->group = ownedJob->group;

               if (queued->group)
               {
                    queued->group->AddJob();
               }

               jobQueue.Push(std::move(queued));
          }
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