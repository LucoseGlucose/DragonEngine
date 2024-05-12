#include "stdafx.h"
#include "RendererComponent.h"

#include "Rendering.h"

RendererComponent::RendererComponent(SceneObject* owner) : Component(owner)
{
	shaderDefaultFunc = GetLitShaderDefaultFunc();
	shaderParamFunc = GetLitShaderParamFunc();
}

RendererComponent::~RendererComponent()
{
	delete mesh;
	delete material;
}

ShaderParamFunc RendererComponent::GetLitShaderDefaultFunc()
{
	return [](RendererComponent* renderer)
	{
		renderer->material->SetParameter("p_albedo", XMFLOAT4(1.f, 1.f, 1.f, 1.f));
		renderer->material->SetParameter("p_roughness", .5f);
		renderer->material->SetParameter("p_metallic", .5f);
		renderer->material->SetParameter("p_normalStrength", 1.f);
		renderer->material->SetParameter("p_aoStrength", 1.f);
		renderer->material->SetParameter("p_emissive", XMFLOAT3(0.f, 0.f, 0.f));
		renderer->material->SetParameter("p_ambientColor", XMFLOAT3(.25f, .25f, .25f));
	};
}

ShaderParamFunc RendererComponent::GetLitShaderParamFunc()
{
	return [](RendererComponent* renderer)
	{
		XMFLOAT4X4 projMat = Rendering::outputCam->GetProjectionMat();
		XMFLOAT4X4 viewMat = Rendering::outputCam->GetViewMat();
		XMFLOAT4X4 modelMat = renderer->GetTransform()->GetMatrix();

		XMMATRIX modelMatMat = DirectX::XMLoadFloat4x4(&modelMat);
		XMMATRIX multiplied = modelMatMat * DirectX::XMLoadFloat4x4(&viewMat) * DirectX::XMLoadFloat4x4(&projMat);

		XMFLOAT4X4 mvpMat;
		DirectX::XMStoreFloat4x4(&mvpMat, DirectX::XMMatrixTranspose(multiplied));

		renderer->material->SetParameter("p_mvpMat", &mvpMat);

		XMFLOAT4X4 shaderModelMat;
		DirectX::XMStoreFloat4x4(&shaderModelMat, DirectX::XMMatrixTranspose(modelMatMat));

		renderer->material->SetParameter("p_modelMat", &shaderModelMat);

		XMFLOAT3 cameraPosition = Rendering::outputCam->GetTransform()->GetPosition();
		renderer->material->SetParameter("p_cameraPosition", &cameraPosition);

		std::vector<LightComponent*> lights = Rendering::scenePass->lights;
		std::map<LightComponent*, float> distances{};

		for (size_t i = 0; i < lights.size(); i++)
		{
			distances[lights[i]] = lights[i]->GetTransform()->GetDistance(renderer->GetTransform(), true);
		}

		std::sort(lights.begin(), lights.end(), [distances](LightComponent* l1, LightComponent* l2)
			{ return distances.at(l1) < distances.at(l2); });

		LightData lightData[5]{};
		for (size_t i = 0; i < 5; i++)
		{
			LightData data{};

			if (i >= lights.size())
			{
				data.color = XMFLOAT3(0.f, 0.f, 0.f);

				lightData[i] = data;
				continue;
			}

			LightComponent* light = lights[i];
			light->GetLightData(&data);

			lightData[i] = data;
		}

		renderer->material->SetParameter<LightData>("p_lights", lightData);
		renderer->material->SetParameter("p_ambientColor", Rendering::scenePass->skyboxObj->ambientColor);

		renderer->material->SetTexture("t_irradiance", Rendering::scenePass->skyboxObj->irradiance);
		renderer->material->SetTexture("t_specularReflections", Rendering::scenePass->skyboxObj->specular);
	};
}

void RendererComponent::SetMaterial(Material* material)
{
	this->material = material;
	if (shaderDefaultFunc != nullptr) shaderDefaultFunc(this);
}

Material* RendererComponent::GetMaterial()
{
	return material;
}

void RendererComponent::SetMesh(Mesh* mesh)
{
	this->mesh = mesh;
}

Mesh* RendererComponent::GetMesh()
{
	return mesh;
}

void RendererComponent::Render(CommandRecorder* recorder)
{
	if (mesh == nullptr || material == nullptr) return;

	shaderParamFunc(this);

	material->Bind(recorder);
	mesh->Draw(recorder);
}