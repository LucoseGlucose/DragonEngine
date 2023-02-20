#pragma once

class TimeManager
{
	static inline double deltaTime{};

public:
	static void Update(double delta);
	static double GetTotalTime();
	static double GetDeltaTime();
};