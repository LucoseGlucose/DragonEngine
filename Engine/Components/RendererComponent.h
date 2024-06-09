#pragma once

#include "Component.h"
#include "Mesh.h"
#include "Material.h"

class RendererComponent : public Component
{
	Mesh* mesh;
	Material* material;

public:
	RendererComponent(SceneObject* owner);
	~RendererComponent();

	BoundingBox aabb;
	BoundingOrientedBox obb;

	virtual void SetMaterial(Material* material);
	GETTER(GetMaterial, material);

	virtual void SetMesh(Mesh* mesh);
	GETTER(GetMesh, mesh);

	virtual void SetMaterialParameters();
	virtual void SetMVPMatrix();
	virtual void SetLightData();

	virtual void Render(CommandRecorder* recorder, PipelineProfile profile);
	virtual void CalculateBoundingBoxes();
};