#include "stdafx.h"

#include "Application.h"
#include "SceneManager.h"
#include "RendererComponent.h"
#include "Material.h"
#include "Texture2D.h"
#include "Rendering.h"
#include "CameraControllerComponent.h"
#include "SkyboxObject.h"
#include "TextureCubemap.h"
#include "PointLightComponent.h"
#include "DirectionalLightComponent.h"
#include "BuildLayer.h"
#include "SceneLayer.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	Application::Init();
	Application::PushLayer(new BuildLayer());

	Scene* scene = new Scene("MainScene");

	CameraComponent* cam = scene->AddObject(new SceneObject("Camera"))->AddComponent<CameraComponent>();
	Rendering::outputCam = cam;
	cam->GetOwner()->AddComponent<CameraControllerComponent>();

	cam->GetTransform()->SetPosition(XMFLOAT3(0.f, 0.f, -4.f));

	LightComponent* light = scene->AddObject(new SceneObject("Light"))->AddComponent<DirectionalLightComponent>();
	light->GetTransform()->SetPosition(XMFLOAT3(2.f, 2.5f, -1.5f));
	light->GetTransform()->SetEulerAngles(XMFLOAT3(28.f, 53.f, 0.f));

	Mesh* cubeMesh = new Mesh(Utils::GetPathFromProject("Models/Cube.fbx"));

	RendererComponent* mesh = scene->AddObject(new SceneObject("Mesh"))->AddComponent<RendererComponent>();
	mesh->mesh = cubeMesh;
	mesh->material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("LitV.cso"),
		Utils::GetPathFromExe("LitP.cso"), Rendering::scenePass->outputFB));

	mesh->material->SetParameter("p_albedo", (void*)&DirectX::Colors::Firebrick.f[0], sizeof(float) * 4);
	mesh->material->SetParameter("p_metallic", .25f);
	mesh->material->SetParameter("p_roughness", .75f);

	SceneManager::AddScene(scene);

	Application::PushLayer(new SceneLayer());
	Application::Run();
}