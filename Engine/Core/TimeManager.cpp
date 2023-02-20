#include "stdafx.h"
#include "TimeManager.h"
#include <GLFW/glfw3.h>

void TimeManager::Update(double delta)
{
	deltaTime = delta;
}

double TimeManager::GetTotalTime()
{
	return glfwGetTime();
}

double TimeManager::GetDeltaTime()
{
	return deltaTime;
}
