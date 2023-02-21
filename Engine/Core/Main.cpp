#include "stdafx.h"

#include "Application.h"
#include "SceneManager.h"
#include "RendererComponent.h"
#include "Material.h"
#include "Texture2D.h"
#include "Rendering.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	std::function<Scene*(void)> func = [&]()
	{
		Scene* scene = new Scene("MainScene");

		CameraComponent* cam = scene->AddObject(new SceneObject("Camera"))->AddComponent<CameraComponent>();
		Rendering::outputCam = cam;

		RendererComponent* quad = scene->AddObject(new SceneObject("Quad"))->AddComponent<RendererComponent>();
		quad->mesh = new Mesh(Utils::GetPathFromProject("Models/Quad.fbx"));
		quad->material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("LitVertex.cso"), Utils::GetPathFromExe("LitPixel.cso"),
			Rendering::sceneFB->colorTexture->samples, Rendering::sceneFB->colorTexture->format));

		quad->GetOwner()->GetTransform()->SetPosition(XMFLOAT3(0, 0, 3.5f));

		Texture2D* tex = Texture2D::Import(Utils::GetPathFromProject("Images/UV Checker.png"), false);
		quad->material->SetTexture("t", tex);
		quad->material->SetSampler("s", Utils::GetDefaultSampler());

		return scene;
	};

	Application::Run(func);
}