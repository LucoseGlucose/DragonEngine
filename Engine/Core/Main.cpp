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

		RendererComponent* cube = scene->AddObject(new SceneObject("Cube"))->AddComponent<RendererComponent>();
		cube->mesh = new Mesh(Utils::GetPathFromProject("Models/Cube.fbx"));
		cube->material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("EquirectToCubemapVertex.cso"), Utils::GetPathFromExe("EquirectToCubemapPixel.cso"),
			Rendering::sceneFB->colorTexture->samples, Rendering::sceneFB->colorTexture->format));

		cube->GetOwner()->GetTransform()->SetPosition(XMFLOAT3(0, 0, 3.5f));

		Texture2D* tex = Texture2D::ImportHDR(Utils::GetPathFromProject("Images/limpopo_golf_course_4k.hdr"));
		cube->material->SetTexture("t_texture", tex);
		cube->material->SetSampler("s_sampler", Utils::GetDefaultSampler());

		RendererComponent* skybox = scene->AddObject(new SkyboxObject("Skybox"))->GetComponent<RendererComponent>();
		skybox->material->SetSampler("s_sampler", Utils::GetDefaultSampler());

		std::array<std::filesystem::path, 6> paths =
		{
			Utils::GetPathFromProject("Images/Six sided skybox/right.jpg"),
			Utils::GetPathFromProject("Images/Six sided skybox/left.jpg"),
			Utils::GetPathFromProject("Images/Six sided skybox/top.jpg"),
			Utils::GetPathFromProject("Images/Six sided skybox/bottom.jpg"),
			Utils::GetPathFromProject("Images/Six sided skybox/front.jpg"),
			Utils::GetPathFromProject("Images/Six sided skybox/back.jpg"),
		};

		TextureCubemap* cubemap = TextureCubemap::Import(paths, true);
		skybox->material->SetTexture("t_texture", cubemap);

		return scene;
	};

	Application::Run(func);
}