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

		Mesh* cubeMesh = new Mesh(Utils::GetPathFromProject("Models/Sphere.fbx"));
		
		for (float x = -5.f; x < 5.f; x += 2.f)
		{
			for (float y = -5.f; y < 5.f; y += 2.f)
			{
				for (float z = -5.f; z < 5.f; z += 2.f)
				{
					RendererComponent* mesh = scene->AddObject(new SceneObject("Mesh"))->AddComponent<RendererComponent>();
					mesh->mesh = cubeMesh;
					mesh->material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("LitV.cso"),
						Utils::GetPathFromExe("LitP.cso"), Rendering::sceneFB));
					mesh->GetTransform()->SetPosition(XMFLOAT3(x, y, z));

					mesh->material->SetParameter("p_albedo", XMFLOAT4(.15f, .25f, .75f, 1.f));
					mesh->material->SetParameter("p_metallic", .8f);
					mesh->material->SetParameter("p_roughness", .35f);
				}
			}
		}

		return scene;
	};

	Application::Run(func);
}