#pragma once

#include <functional>

class JobGroup;

class Job
{
public:
	std::function<void()> function;
	JobGroup* group = nullptr;

	void Execute();
};