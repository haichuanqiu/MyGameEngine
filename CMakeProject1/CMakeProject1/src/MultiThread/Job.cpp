

#include "Job.h"
#include "JobGroup.h"
#include "Profiler/profiler.h"

void Job::Execute()
{

          function();
     

     // 告诉 JobGroup：
     // “我完成了”
     if (group != nullptr)
     {
          group->JobFinished();
     }
}
