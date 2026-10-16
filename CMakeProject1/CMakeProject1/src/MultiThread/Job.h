#pragma once

#include <functional>
#include <iostream>
class JobGroup;

struct Job
{
     JobGroup* group = nullptr;

     virtual ~Job() = default;

     virtual void ToExecute() = 0;

};



struct JobParallelFor
{
     JobGroup* group = nullptr;
     int batchSize;
     virtual ~JobParallelFor() = default;

     virtual void ToExecute(int index) = 0;

};