#pragma once

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

class Layer
{
public:
	virtual void OnPush();
	virtual void Update();
	virtual void Resize(Vector2 newSize);
	virtual void OnPop();
};