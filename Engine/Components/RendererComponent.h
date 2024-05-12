#pragma once

#include "Component.h"
#include "Mesh.h"
#include "Material.h"

typedef void(*ShaderParamFunc)(class RendererComponent* renderer);

class RendererComponent : public Component
{
	Mesh* mesh;
	Material* material;

public:
	RendererComponent(SceneObject* owner);
	~RendererComponent();

	ShaderParamFunc shaderParamFunc;
	ShaderParamFunc shaderDefaultFunc;

	static ShaderParamFunc GetLitShaderDefaultFunc();
	static ShaderParamFunc GetLitShaderParamFunc();

	virtual void SetMaterial(Material* material);
	virtual Material* GetMaterial();

	virtual void SetMesh(Mesh* mesh);
	virtual Mesh* GetMesh();

	virtual void Render(CommandRecorder* recorder);
};