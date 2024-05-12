#include "stdafx.h"
#include "Game.h"

#include "RendererComponent.h"
#include "Material.h"
#include "Texture2D.h"
#include "Rendering.h"
#include "CameraControllerComponent.h"
#include "SkyboxObject.h"
#include "TextureCubemap.h"
#include "PointLightComponent.h"
#include "DirectionalLightComponent.h"

Scene* Game::Init()
{
	Scene* scene = new Scene("MainScene");

	CameraComponent* cam = scene->AddObject(new SceneObject("Camera"))->AddComponent<CameraComponent>();
	Rendering::outputCam = cam;
	cam->GetOwner()->AddComponent<CameraControllerComponent>();

	cam->GetTransform()->SetPosition(XMFLOAT3(0.f, 0.f, -4.f));

	LightComponent* light = scene->AddObject(new SceneObject("Light"))->AddComponent<DirectionalLightComponent>();
	light->strength = .5f;
	light->color = XMFLOAT3(1.f, .99f, .97f);
	light->GetTransform()->SetPosition(XMFLOAT3(2.f, 2.5f, -1.5f));
	light->GetTransform()->SetEulerAngles(XMFLOAT3(28.f, 53.f, 0.f));

	Mesh* mesh = new Mesh(Utils::GetPathFromProject("Models/Cube.fbx"));

	RendererComponent* object = scene->AddObject(new SceneObject("Mesh"))->AddComponent<RendererComponent>();
	object->SetMesh(mesh);
	object->SetMaterial(new Material(ShaderProgram::Create(Utils::GetPathFromExe("LitV.cso"),
		Utils::GetPathFromExe("LitP.cso"), Rendering::scenePass->outputFB)));

	object->GetMaterial()->SetParameter("p_albedo", (void*)&DirectX::Colors::DodgerBlue.f[0], sizeof(float) * 4);
	object->GetMaterial()->SetParameter("p_metallic", .7f);
	object->GetMaterial()->SetParameter("p_roughness", .1f);

	return scene;
}
