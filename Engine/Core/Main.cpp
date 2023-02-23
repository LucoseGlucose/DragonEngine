#include "stdafx.h"

#include "Application.h"
#include "SceneManager.h"
#include "RendererComponent.h"
#include "Material.h"
#include "Texture2D.h"
#include "Rendering.h"
#include "CameraControllerComponent.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	std::function<Scene*(void)> func = [&]()
	{
		Scene* scene = new Scene("MainScene");

		CameraComponent* cam = scene->AddObject(new SceneObject("Camera"))->AddComponent<CameraComponent>();
		Rendering::outputCam = cam;
		cam->GetOwner()->AddComponent<CameraControllerComponent>();

		RendererComponent* cube = scene->AddObject(new SceneObject("Cube"))->AddComponent<RendererComponent>();
		cube->mesh = new Mesh(Utils::GetPathFromProject("Models/Cube.fbx"));
		cube->material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("LitVertex.cso"), Utils::GetPathFromExe("LitPixel.cso"),
			Rendering::sceneFB->colorTexture->samples, Rendering::sceneFB->colorTexture->format));

		cube->GetOwner()->GetTransform()->SetPosition(XMFLOAT3(0, 0, 3.5f));

		Texture2D* tex = Texture2D::Import(Utils::GetPathFromProject("Images/UV Checker.png"), false, true);
		cube->material->SetTexture("t", tex);
		cube->material->SetSampler("s", Utils::GetDefaultSampler());

		return scene;
	};

	Application::Run(func);
}