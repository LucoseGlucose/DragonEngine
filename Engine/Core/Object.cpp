#include "stdafx.h"
#include "Object.h"

#include <random>

Object::Object(std::string name)
{
	this->name = name;

	int first = std::rand();
	int second = std::rand();
	id = second;
	id << 32;
	id |= first;
}
