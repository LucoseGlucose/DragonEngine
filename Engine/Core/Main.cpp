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
		light->GetTransform()->SetEulerAngles(XMFLOAT3(28.f, 53.f, 0.f));

		Texture2D* bricks = Texture2D::Import(Utils::GetPathFromProject("Images/brickwall.jpg"), true, true);
		Texture2D* normal = Texture2D::Import(Utils::GetPathFromProject("Images/brickwall_normal.jpg"), false, true);

		Sampler sampler = Utils::GetDefaultSampler();
		sampler.Filter = D3D12_FILTER_ANISOTROPIC;

		float metallic = 0.f;
		float roughness = .95f;

		Mesh* cubeMesh = new Mesh(Utils::GetPathFromProject("Models/Cube.fbx"));

		for (float x = -10; x < 10; x += 2)
		{
			for (float y = -10; y < 10; y += 2)
			{
				RendererComponent* mesh = scene->AddObject(new SceneObject("Mesh"))->AddComponent<RendererComponent>();
				mesh->mesh = cubeMesh;
				mesh->material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("LitVertex.cso"), Utils::GetPathFromExe("LitPixel.cso"),
					Rendering::sceneFB->colorTexture->samples, Rendering::sceneFB->colorTexture->format));

				mesh->GetTransform()->SetPosition(XMFLOAT3(x, y, 0.f));

				mesh->material->SetSampler("s_sampler", sampler);
				mesh->material->SetTexture("t_albedoW", bricks);
				mesh->material->SetTexture("t_normalN", normal);

				mesh->material->SetParameter("p_metallic", &metallic, sizeof(float));
				mesh->material->SetParameter("p_roughness", &roughness, sizeof(float));
			}
		}

		return scene;
	};

	Application::Run(func);
}