#pragma once

#include "Component.h"
#include "Mesh.h"
#include "Material.h"

class RendererComponent : public Component
{
public:
	RendererComponent(SceneObject* owner);
	~RendererComponent();

	Mesh* mesh;
	Material* material;

	std::function<void(RendererComponent*)> shaderParamFunc;
	static std::function<void(RendererComponent*)> GetLitShaderParamFunc();

	virtual void Render(CommandRecorder* recorder);
};