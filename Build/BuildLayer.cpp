#include "stdafx.h"
#include "BuildLayer.h"

#include "Rendering.h"
#include "RayTracingRenderPass.h"

void BuildLayer::OnPush()
{
	RayTracingRenderPass* rtPass = new RayTracingRenderPass();
	Rendering::renderPasses.insert(Rendering::renderPasses.begin() + 2, rtPass);

	rtPass->CreateBuffers();
}

void BuildLayer::Update()
{
	
}

void BuildLayer::Resize(Vector2 newSize)
{
	Rendering::Resize(newSize);
}

void BuildLayer::OnPop()
{

}
