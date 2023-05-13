#include "stdafx.h"
#include "RenderPass.h"

#include "Rendering.h"

RenderPass::~RenderPass()
{
	delete outputFB;
}

void RenderPass::Render(Framebuffer* inputFB)
{
	CommandRecorder* recorder = Rendering::GetRecorder();
	recorder->StartRecording();

	Execute(inputFB, recorder);

	recorder->Execute();
	Rendering::RecycleRecorder(recorder);
}

void RenderPass::Execute(Framebuffer* inputFB, CommandRecorder* recorder)
{

}

void RenderPass::Resize(Framebuffer* inputFB, XMUINT2 newSize)
{
	outputFB->Resize(newSize);
}
