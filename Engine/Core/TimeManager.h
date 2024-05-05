#pragma once

class TimeManager
{
	static inline double deltaTime{};
	static inline double totalTime{};

public:
	static void Update(double delta);
	static double GetTotalTime();
	static double GetDeltaTime();
};