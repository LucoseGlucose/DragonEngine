#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Layer
{
public:
	virtual void OnPush();
	virtual void Update();
	virtual void Resize(XMUINT2 newSize);
	virtual void OnPop();
};