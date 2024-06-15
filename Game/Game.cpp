#include "stdafx.h"
#include "Game.h"

#include "RendererComponent.h"
#include "Material.h"
#include "Rendering.h"
#include "CameraControllerComponent.h"
#include "DirectionalLightComponent.h"

Scene* Game::Init()
{
	Scene* scene = new Scene("MainScene");

	CameraComponent* cam = scene->AddObject(new SceneObject("Camera"))->AddComponent<CameraComponent>();
	Rendering::outputCam = cam;
	cam->GetOwner()->AddComponent<CameraControllerComponent>();

	cam->GetTransform()->SetPosition(Vector3(0.f, 0.f, -4.f));

	LightComponent* light = scene->AddObject(new SceneObject("Light"))->AddComponent<DirectionalLightComponent>();
	light->strength = .5f;
	light->color = Vector3(1.f, .99f, .97f);
	light->GetTransform()->SetPosition(Vector3(2.f, 2.5f, -1.5f));
	light->GetTransform()->SetEulerAngles(Vector3(28.f, 53.f, 0.f));

	Mesh* mesh = new Mesh(Utils::GetPathFromProject("Models/Sphere.fbx"));

	RendererComponent* object = scene->AddObject(new SceneObject("Mesh"))->AddComponent<RendererComponent>();
	object->SetMesh(mesh);
	object->SetMaterial(new Material(ShaderProgram::Create("LitV.cso", "LitP.cso")));
	
	object->GetMaterial()->SetParameter("p_albedo", Color(DirectX::ColorsLinear::DodgerBlue));
	object->GetMaterial()->SetParameter("p_metallic", .9f);
	object->GetMaterial()->SetParameter("p_roughness", .3f);

	return scene;
}
