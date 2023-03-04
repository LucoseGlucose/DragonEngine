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

		RendererComponent* mesh = scene->AddObject(new SceneObject("Mesh"))->AddComponent<RendererComponent>();
		mesh->mesh = new Mesh(Utils::GetPathFromProject("Models/Cube.fbx"));
		mesh->material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("LitVertex.cso"), Utils::GetPathFromExe("LitPixel.cso"),
			Rendering::sceneFB->colorTexture->samples, Rendering::sceneFB->colorTexture->format));

		Sampler sampler = Utils::GetDefaultSampler();
		sampler.Filter = D3D12_FILTER_ANISOTROPIC;
		mesh->material->SetSampler("s_sampler", sampler);

		Texture2D* bricks = Texture2D::Import(Utils::GetPathFromProject("Images/brickwall.jpg"), true, true);
		mesh->material->SetTexture("t_albedoW", bricks);

		Texture2D* normal = Texture2D::Import(Utils::GetPathFromProject("Images/brickwall_normal.jpg"), false, true);
		mesh->material->SetTexture("t_normalN", normal);

		float metallic = 0.f;
		mesh->material->SetParameter("p_metallic", &metallic, sizeof(float));

		float roughness = .95f;
		mesh->material->SetParameter("p_roughness", &roughness, sizeof(float));

		RendererComponent* skybox = scene->AddObject(new SkyboxObject("Skybox"))->GetComponent<RendererComponent>();

		TextureCubemap* cubemap = TextureCubemap::ImportHDR(Utils::GetPathFromProject("Images/limpopo_golf_course_4k.hdr"), true);
		skybox->material->SetTexture("t_texture", cubemap);

		TextureCubemap* irradiance = TextureCubemap::ComputeDiffuseIrradiance(cubemap, XMUINT2(32, 32));
		mesh->material->SetTexture("t_irradiance", irradiance);

		TextureCubemap* specular = TextureCubemap::ComputeAmbientSpecular(cubemap, XMUINT2(256, 256), 5);
		mesh->material->SetTexture("t_specularReflections", specular);

		Texture2D* brdf = Texture2D::Import(Utils::GetPathFromProject("Images/BRDF LUT.png"), false, false);
		mesh->material->SetTexture("t_brdfLUT", brdf);

		return scene;
	};

	Application::Run(func);
}