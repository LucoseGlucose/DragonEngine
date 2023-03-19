#pragma once

#include "Component.h"
#include "Mesh.h"
#include "Material.h"

typedef void(*ShaderParamFunc)(class RendererComponent* renderer);

class RendererComponent : public Component
{
public:
	RendererComponent(SceneObject* owner);
	~RendererComponent();

	Mesh* mesh;
	Material* material;

	ShaderParamFunc shaderParamFunc;
	static ShaderParamFunc GetLitShaderParamFunc();

	virtual void Render(CommandRecorder* recorder);
};