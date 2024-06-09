#pragma once

#include "Layer.h"

class SceneLayer : public Layer
{
	double lastFrameTime;

public:
	virtual void OnPush() override;
	virtual void Update() override;
	virtual void Resize(Vector2 newSize) override;
	virtual void OnPop() override;
};