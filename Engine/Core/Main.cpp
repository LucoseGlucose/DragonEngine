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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	std::function<Scene*(void)> func = [&]()
	{
		Scene* scene = new Scene("MainScene");

		CameraComponent* cam = scene->AddObject(new SceneObject("Camera"))->AddComponent<CameraComponent>();
		Rendering::outputCam = cam;
		cam->GetOwner()->AddComponent<CameraControllerComponent>();

		cam->GetTransform()->SetPosition(XMFLOAT3(0.f, 0.f, -4.f));

		LightComponent* light = scene->AddObject(new SceneObject("Light"))->AddComponent<DirectionalLightComponent>();
		light->GetTransform()->SetPosition(XMFLOAT3(2.f, 2.5f, -1.5f));
		light->GetTransform()->SetEulerAngles(XMFLOAT3(55.f, 45.f, 0.f));
		light->strength = 3.f;

		RendererComponent* mesh = scene->AddObject(new SceneObject("Mesh"))->AddComponent<RendererComponent>();
		mesh->mesh = new Mesh(Utils::GetPathFromProject("Models/Smooth Monkey.fbx"));
		mesh->material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("LitVertex.cso"), Utils::GetPathFromExe("LitPixel.cso"),
			Rendering::sceneFB->colorTexture->samples, Rendering::sceneFB->colorTexture->format));

		float metallic = .1f;
		mesh->material->SetParameter("p_metallic", &metallic, sizeof(float));

		float roughness = .9f;
		mesh->material->SetParameter("p_roughness", &roughness, sizeof(float));

		RendererComponent* skybox = scene->AddObject(new SkyboxObject("Skybox"))->GetComponent<RendererComponent>();

		TextureCubemap* cubemap = TextureCubemap::ImportHDR(Utils::GetPathFromProject("Images/limpopo_golf_course_4k.hdr"));
		skybox->material->SetTexture("t_texture", cubemap);

		return scene;
	};

	Application::Run(func);
}