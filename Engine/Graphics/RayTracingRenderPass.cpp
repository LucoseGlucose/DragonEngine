#include "stdafx.h"
#include "RayTracingRenderPass.h"

#include "Rendering.h"
#include "SceneManager.h"

RayTracingRenderPass::RayTracingRenderPass() : ProcessRenderPass(new Material(ShaderProgram::Create("OutputV.cso", "RayTracingP.cso")))
{

}

RayTracingRenderPass::~RayTracingRenderPass()
{
	delete meshBuffer;
	delete indicesBuffer;
	delete verticesBuffer;

	delete meshBufferUpload;
	delete indicesBufferUpload;
	delete verticesBufferUpload;
}

struct RTMesh
{
	UINT32 indexCount;
	UINT32 vertexCount;
	float roughness;
	float metallic;
	Vector4 color;
};

void RayTracingRenderPass::CreateBuffers()
{
	CommandRecorder* recorder = Rendering::GetRecorder();
	recorder->StartRecording();

	std::vector<RendererComponent*> renderers = SceneManager::GetActiveScene()->FindComponents<RendererComponent>();
	std::vector<Mesh*> meshes;

	std::vector<RTMesh> meshData;
	std::vector<UINT32> indices;
	std::vector<VertexData> vertices;

	for (size_t r = 0; r < renderers.size(); r++)
	{
		Mesh* mesh = renderers[r]->GetMesh();
		meshes.push_back(mesh);

		meshData.push_back(RTMesh{ static_cast<UINT32>(mesh->indices.size()),
			static_cast<UINT32>(mesh->vertices.size()), .5f, .5f, Vector4(0, 0, 1, 1) });

		for (size_t i = 0; i < mesh->indices.size(); i++)
		{
			indices.push_back(mesh->indices[i]);
		}

		for (size_t i = 0; i < mesh->vertices.size(); i++)
		{
			vertices.push_back(mesh->vertices[i]);
		}
	}

	meshBuffer = new Buffer(meshData.size() * sizeof(RTMesh), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_FLAG_NONE);

	meshBuffer->uavDesc.Buffer.StructureByteStride = sizeof(RTMesh);
	meshBuffer->uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	meshBuffer->uavDesc.Buffer.NumElements = meshData.size();

	indicesBuffer = new Buffer(indices.size() * sizeof(UINT32), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_FLAG_NONE);

	indicesBuffer->uavDesc.Format = DXGI_FORMAT_R32_UINT;
	indicesBuffer->uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	indicesBuffer->uavDesc.Buffer.NumElements = indices.size();

	verticesBuffer = new Buffer(vertices.size() * sizeof(VertexData), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_FLAG_NONE);

	verticesBuffer->uavDesc.Buffer.StructureByteStride = sizeof(VertexData);
	verticesBuffer->uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	verticesBuffer->uavDesc.Buffer.NumElements = vertices.size();

	meshBufferUpload = new Buffer(meshBuffer->size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_COMMON);
	indicesBufferUpload = new Buffer(indicesBuffer->size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_COMMON);
	verticesBufferUpload = new Buffer(verticesBuffer->size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_COMMON);

	meshBuffer->UploadData(recorder, meshBufferUpload, meshData.data());
	indicesBuffer->UploadData(recorder, indicesBufferUpload, indices.data());
	verticesBuffer->UploadData(recorder, verticesBufferUpload, vertices.data());

	auto mt = meshBuffer->TransitionToState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	auto it = indicesBuffer->TransitionToState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	auto vt = verticesBuffer->TransitionToState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	Rendering::RecordBarriers(recorder, { mt, it, vt });

	recorder->Execute();
	Rendering::RecycleRecorder(recorder);
}

void RayTracingRenderPass::Execute(Framebuffer* inputFB, CommandRecorder* recorder)
{
	outputFB->Setup(recorder);

	material->SetParameter("p_fieldOfView", Rendering::outputCam->GetFOV());
	material->SetParameter("p_nearClip", Rendering::outputCam->GetNear());
	material->SetParameter("p_farClip", Rendering::outputCam->GetFar());
	material->SetParameter("p_aspectRatio", Rendering::outputCam->GetSize().x / Rendering::outputCam->GetSize().y);
	material->SetParameter("p_cameraPosition", Rendering::outputCam->GetTransform()->GetPosition());
	material->SetParameter("p_viewMat", Rendering::outputCam->GetViewMat().Invert());
	material->SetParameter("p_numBounces", 0);

	material->SetUAV("u_meshes", meshBuffer);
	material->SetUAV("u_indices", indicesBuffer);
	material->SetUAV("u_vertices", verticesBuffer);

	material->SetTexture("t_skybox", Rendering::scenePass->skyboxTexture);
	material->SetSampler("s_skyboxSampler", Rendering::GetDefaultSampler());

	material->Bind(recorder, outputFB->pipelineProfile);
	Rendering::quadMesh->Draw(recorder);
}

void RayTracingRenderPass::Resize(Framebuffer* inputFB, Vector2 newSize)
{
	outputFB->Resize(newSize);
}
