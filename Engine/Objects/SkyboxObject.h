#pragma once

#include "SceneObject.h"
#include "Mesh.h"

class SkyboxObject : public SceneObject
{
public:
	SkyboxObject(std::string name);

	static inline Mesh* skyboxMesh{};
};