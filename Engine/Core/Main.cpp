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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	std::function<Scene*(void)> func = [&]()
	{
		Scene* scene = new Scene("MainScene");

		CameraComponent* cam = scene->AddObject(new SceneObject("Camera"))->AddComponent<CameraComponent>();
		Rendering::outputCam = cam;
		cam->GetOwner()->AddComponent<CameraControllerComponent>();

		cam->GetTransform()->SetPosition(XMFLOAT3(0.f, 0.f, -4.f));

		RendererComponent* cube = scene->AddObject(new SceneObject("Cube"))->AddComponent<RendererComponent>();
		cube->mesh = new Mesh(Utils::GetPathFromProject("Models/Cube.fbx"));
		cube->material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("LitVertex.cso"), Utils::GetPathFromExe("LitPixel.cso"),
			Rendering::sceneFB->colorTexture->samples, Rendering::sceneFB->colorTexture->format));

		XMFLOAT4 col = XMFLOAT4(.2f, .4f, 1.f, 1.f);
		cube->material->SetParameter("p_albedo", &col, sizeof(col));

		RendererComponent* skybox = scene->AddObject(new SkyboxObject("Skybox"))->GetComponent<RendererComponent>();

		TextureCubemap* cubemap = TextureCubemap::ImportHDR(Utils::GetPathFromProject("Images/limpopo_golf_course_4k.hdr"));
		skybox->material->SetTexture("t_texture", cubemap);

		return scene;
	};

	Application::Run(func);
}