#include "stdafx.h"
#include "BuildLayer.h"

#include "Rendering.h"

void BuildLayer::OnPush()
{

}

void BuildLayer::Update()
{
	
}

void BuildLayer::Resize(XMUINT2 newSize)
{
	Rendering::Resize(newSize);
}

void BuildLayer::OnPop()
{

}

XMUINT2 BuildLayer::GetViewportSize()
{
	return Application::GetUnsignedFramebufferSize();
}
