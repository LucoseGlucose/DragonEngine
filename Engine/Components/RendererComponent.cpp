#include "stdafx.h"
#include "RendererComponent.h"

#include "Rendering.h"

RendererComponent::RendererComponent(SceneObject* owner) : Component(owner)
{
	shaderParamFunc = Utils::GetLitShaderParamFunc();
}

RendererComponent::~RendererComponent()
{
	delete mesh;
	delete material;
}

void RendererComponent::Render(CommandRecorder* recorder)
{
	if (mesh == nullptr || material == nullptr) return;

	shaderParamFunc(this);

	material->Bind(recorder);
	mesh->Draw(recorder);
}