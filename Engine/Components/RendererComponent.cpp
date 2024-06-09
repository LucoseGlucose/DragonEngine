#include "stdafx.h"
#include "RendererComponent.h"

#include "Rendering.h"

RendererComponent::RendererComponent(SceneObject* owner) : Component(owner)
{
	std::function<void(Matrix)> func = [this](Matrix m) { CalculateBoundingBoxes(); };
	GetTransform()->onMatrixChanged.Subscribe(func);
}

RendererComponent::~RendererComponent()
{
	delete mesh;
	delete material;
}

void RendererComponent::SetMaterial(Material* material)
{
	this->material = material;
	
	material->SetParameter("p_albedo", Vector4::One);
	material->SetParameter("p_roughness", .5f);
	material->SetParameter("p_metallic", .5f);
	material->SetParameter("p_normalStrength", 1.f);
	material->SetParameter("p_aoStrength", 1.f);
	material->SetParameter("p_emissive", Vector3::Zero);
	material->SetParameter("p_ambientColor", Vector3::One);
}

void RendererComponent::SetMesh(Mesh* mesh)
{
	this->mesh = mesh;
}

void RendererComponent::SetMaterialParameters()
{
	if (material->cbParameters.contains("p_mvpMat")) SetMVPMatrix();

	Matrix modelMat = GetTransform()->GetMatrix();
	material->SetParameter("p_modelMat", &modelMat);

	Vector3 cameraPosition = Rendering::outputCam->GetTransform()->GetPosition();
	material->SetParameter("p_cameraPosition", &cameraPosition);

	if (material->cbParameters.contains("p_lights")) SetLightData();

	material->SetTexture("t_irradiance", Rendering::scenePass->diffuseSkybox);
	material->SetTexture("t_specularReflections", Rendering::scenePass->specularSkybox);
}

void RendererComponent::SetMVPMatrix()
{
	Matrix projMat = Rendering::outputCam->GetProjectionMat();
	Matrix viewMat = Rendering::outputCam->GetViewMat();
	Matrix modelMat = GetTransform()->GetMatrix();

	Matrix multiplied = modelMat * viewMat * projMat;
	Matrix transpose = multiplied.Transpose();

	material->SetParameter("p_mvpMat", &transpose);
}

void RendererComponent::SetLightData()
{
	std::vector<LightComponent*> lights = Rendering::scenePass->lights;
	std::map<LightComponent*, float> distances{};

	for (size_t i = 0; i < lights.size(); i++)
	{
		distances[lights[i]] = lights[i]->GetTransform()->GetDistance(GetTransform());
	}

	std::sort(lights.begin(), lights.end(), [distances](LightComponent* l1, LightComponent* l2)
		{ return distances.at(l1) < distances.at(l2); });

	LightData lightData[5]{};
	for (size_t i = 0; i < 5; i++)
	{
		LightData data{};

		if (i >= lights.size())
		{
			data.color = Vector3(0.f, 0.f, 0.f);

			lightData[i] = data;
			continue;
		}

		LightComponent* light = lights[i];
		light->GetLightData(&data);

		lightData[i] = data;
	}

	material->SetParameter<LightData>("p_lights", lightData);
}

void RendererComponent::Render(CommandRecorder* recorder, PipelineProfile profile)
{
	if (mesh == nullptr) return;

	if (material != nullptr)
	{
		SetMaterialParameters();
		material->Bind(recorder, profile);
	}
	else
	{
		material = Rendering::missingMaterial;
		SetMVPMatrix();

		material->Bind(recorder, profile);
		material = nullptr;
	}

	mesh->Draw(recorder);
}

void RendererComponent::CalculateBoundingBoxes()
{
	Vector3 pos = GetTransform()->GetPosition();

}
