#pragma once

#include <map>
#include <functional>

template<typename T>
struct Subscription
{
	std::function<void(T)> func;
	int token;

	Subscription(const std::function<void(T)>& func) : func(func), token(0)
	{

	}
};

template<typename T>
struct Event
{
private:

	std::map<UINT32, std::function<void(T)>> subscribers;

public:

	void operator()(T param)
	{
		Invoke(param);
	}

	void Invoke(T param)
	{
		for (std::pair<int, std::function<void(T)>> pair : subscribers)
		{
			pair.second(param);
		}
	}

	void Subscribe(Subscription<T>* sub)
	{
		if (sub->token != 0) Utils::CrashWithMessage(L"Subscription is already subscribed to an event!");

		sub->token = std::rand();
		subscribers.emplace(sub->token, sub->func);
	}

	int Subscribe(const std::function<void(T)>& func)
	{
		int token = std::rand();
		subscribers.emplace(token, func);

		return token;
	}

	void Unsubscribe(Subscription<T>* sub)
	{
		if (sub->token == 0) return;

		subscribers.erase(sub->token);
		sub->token = 0;
	}

	void Unsubscribe(int token)
	{
		subscribers.erase(token);
	}
};