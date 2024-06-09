#pragma once

#include "Layer.h"

class BuildLayer : public Layer
{
public:
	virtual void OnPush() override;
	virtual void Update() override;
	virtual void Resize(Vector2 newSize) override;
	virtual void OnPop() override;
};