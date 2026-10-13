#include "Job.h"
#include "JobGroup.h"
#include "Profiler/profiler.h"

void Job::Execute()
{

     ToExecute();


     if (group != nullptr)
     {
          group->JobFinished();
     }
}
