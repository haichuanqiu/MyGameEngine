#pragma once
#include "JobGroup.h"
struct QueuedJob
{
     public:
     std::function<void()> function;
     JobGroup* group = nullptr;

     void Execute()
     {
          function();

          if (group)
          {
               group->JobFinished();
          }
     }
};