#pragma once

#include <vector>
#include <functional>

template<typename T>
struct Event
{
private:
	std::list<std::function<void(T)>> subscribers;

public:

	void operator()(T param)
	{
		Invoke(param);
	}

	void Invoke(T param)
	{
		for (const std::function<void(T)>& func : subscribers)
		{
			if (func == nullptr) continue;
			func(param);
		}
	}

	void Subscribe(const std::function<void(T)>& func)
	{
		subscribers.push_back(func);
	}

	void Unsubscribe(const std::function<void(T)>& func)
	{
		subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), func));
	}
};