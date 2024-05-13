#include "stdafx.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include "Rendering.h"

Mesh::Mesh(std::filesystem::path path) : vertices(), indices(), aabb()
{
	Assimp::Importer importer{};
	const aiScene* scene = importer.ReadFile(path.string(), importFlags);

	if (scene->mNumMeshes < 1) Utils::CrashWithMessage(L"There must be exactly one mesh in file!");
	aiMesh* mesh = scene->mMeshes[0];

	aiVector3D center = mesh->mAABB.mMax + mesh->mAABB.mMin;
	aiVector3D extents = mesh->mAABB.mMax - mesh->mAABB.mMin;

	aabb = BoundingBox(XMFLOAT3(center.x, center.y, center.z), XMFLOAT3(extents.x, extents.y, extents.z));

	vertices.resize(mesh->mNumVertices);

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		VertexData vertex{};

		vertex.position = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		vertex.uv = XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		vertex.tangent = XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
		vertex.bitangent = XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);

		vertices[i] = vertex;
	}

	for (size_t f = 0; f < mesh->mNumFaces; f++)
	{
		aiFace face = mesh->mFaces[f];
		for (size_t i = 0; i < face.mNumIndices; i++)
		{
			indices.push_back(face.mIndices[i]);
		}
	}

	UploadMeshData();
}

void Mesh::UploadMeshData()
{
	if (vertices.empty()) Utils::CrashWithMessage(L"Mesh is empty!");

	if (vertexBuffer != nullptr) vertexBuffer->Release();
	if (vertexUploadBuffer != nullptr) vertexUploadBuffer->Release();
	if (indexBuffer != nullptr) indexBuffer->Release();
	if (indexUploadBuffer != nullptr) indexUploadBuffer->Release();

	vertexBuffer.Reset();
	vertexUploadBuffer.Reset();
	indexBuffer.Reset();
	indexUploadBuffer.Reset();

	CommandRecorder* recorder = Rendering::GetRecorder();
	recorder->StartRecording();

	uint32_t vBufferSize = vertices.size() * sizeof(VertexData);

	CD3DX12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC vertexResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer)));
	NAME_D3D_OBJECT(vertexBuffer);

	CD3DX12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexUploadBuffer)));
	NAME_D3D_OBJECT(vertexUploadBuffer);

	D3D12_SUBRESOURCE_DATA vertexData{};
	vertexData.pData = vertices.data();
	vertexData.RowPitch = vBufferSize;
	vertexData.SlicePitch = vBufferSize;

	UpdateSubresources(recorder->list.Get(), vertexBuffer.Get(), vertexUploadBuffer.Get(), 0, 0, 1, &vertexData);

	CD3DX12_RESOURCE_BARRIER vBufferTransition = CD3DX12_RESOURCE_BARRIER::Transition(
		vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	uint32_t iBufferSize = indices.size() * sizeof(uint32_t);

	CD3DX12_RESOURCE_DESC indexResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(iBufferSize);

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE,
		&indexResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&indexBuffer)));
	NAME_D3D_OBJECT(indexBuffer);

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE,
		&indexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexUploadBuffer)));
	NAME_D3D_OBJECT(indexUploadBuffer);

	D3D12_SUBRESOURCE_DATA indexData{};
	indexData.pData = indices.data();
	indexData.RowPitch = iBufferSize;
	indexData.SlicePitch = iBufferSize;

	UpdateSubresources(recorder->list.Get(), indexBuffer.Get(), indexUploadBuffer.Get(), 0, 0, 1, &indexData);

	CD3DX12_RESOURCE_BARRIER iBufferTransition = CD3DX12_RESOURCE_BARRIER::Transition(
		indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	CD3DX12_RESOURCE_BARRIER barriers[] = { vBufferTransition, iBufferTransition };
	recorder->list->ResourceBarrier(_countof(barriers), barriers);

	recorder->Execute();
	Rendering::RecycleRecorder(recorder);

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = vBufferSize;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = iBufferSize;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

void Mesh::Draw(CommandRecorder* recorder)
{
	recorder->list->IASetVertexBuffers(0, 1, &vertexBufferView);
	recorder->list->IASetIndexBuffer(&indexBufferView);
	recorder->list->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);
}
