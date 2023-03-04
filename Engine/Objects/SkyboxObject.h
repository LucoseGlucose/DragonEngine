#pragma once

#include "SceneObject.h"
#include "Mesh.h"
#include "TextureCubemap.h"
#include "RendererComponent.h"

class SkyboxObject : public SceneObject
{
	RendererComponent* renderer;

public:
	SkyboxObject(std::string name);

	static inline Mesh* skyboxMesh{};

	TextureCubemap* skybox = nullptr;
	TextureCubemap* irradiance = nullptr;
	TextureCubemap* specular = nullptr;

	RendererComponent* GetRenderer();
};