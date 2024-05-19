#pragma once

#include <vector>
#include <functional>

template<typename T>
struct Event
{
	using FuncType = std::function<void(T)>;

	void Invoke(T param);
	void Subscribe(FuncType func);
	void Unsubscribe(FuncType func);

private:
	std::list<FuncType> subscribers;
};

template<typename T>
inline void Event<T>::Invoke(T param)
{
	for (FuncType func : subscribers)
	{
		if (func == nullptr) continue;
		func(param);
	}
}

template<typename T>
inline void Event<T>::Subscribe(FuncType func)
{
	subscribers.push_back(func);
}

template<typename T>
inline void Event<T>::Unsubscribe(FuncType func)
{
	subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), func));
}
