#pragma once

#include <functional>
#include <iostream>
class JobGroup;

struct Job
{
     JobGroup* group = nullptr;

     virtual ~Job() = default;

     virtual void ToExecute() = 0;

     void Execute();
};