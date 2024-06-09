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

	aabb = BoundingBox(Vector3(center.x, center.y, center.z), Vector3(extents.x, extents.y, extents.z));

	vertices.resize(mesh->mNumVertices);

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		VertexData vertex{};

		vertex.position = Vector3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.normal = Vector3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		vertex.uv = Vector2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		vertex.tangent = Vector3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
		vertex.bitangent = Vector3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);

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

	vertexBuffer.resourceBuffer.Reset();
	indexBuffer.resourceBuffer.Reset();

	CommandRecorder* recorder = Rendering::GetRecorder();
	recorder->StartRecording();

	vertexBuffer = Buffer(vertices.size() * sizeof(VertexData), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);

	Buffer vertexUploadBuffer = Buffer(vertexBuffer.size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

	D3D12_SUBRESOURCE_DATA vertexData{ vertices.data(), vertexBuffer.size, vertexBuffer.size };
	UpdateSubresources(recorder->list.Get(), vertexBuffer.resourceBuffer.Get(), vertexUploadBuffer.resourceBuffer.Get(), 0, 0, 1, &vertexData);

	vertexBuffer.currentState = D3D12_RESOURCE_STATE_COPY_DEST;
	CD3DX12_RESOURCE_BARRIER vertexBufferTransition = vertexBuffer.TransitionToState(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	indexBuffer = Buffer(indices.size() * sizeof(UINT32), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);

	Buffer indexUploadBuffer = Buffer(indexBuffer.size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

	D3D12_SUBRESOURCE_DATA indexData{ indices.data(), indexBuffer.size, indexBuffer.size };
	UpdateSubresources(recorder->list.Get(), indexBuffer.resourceBuffer.Get(), indexUploadBuffer.resourceBuffer.Get(), 0, 0, 1, &indexData);

	indexBuffer.currentState = D3D12_RESOURCE_STATE_COPY_DEST;
	CD3DX12_RESOURCE_BARRIER indexBufferTransition = indexBuffer.TransitionToState(D3D12_RESOURCE_STATE_INDEX_BUFFER);

	Rendering::RecordBarriers(recorder, { vertexBufferTransition, indexBufferTransition });

	recorder->Execute();
	Rendering::RecycleRecorder(recorder);

	vertexBufferView.BufferLocation = vertexBuffer.resourceBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = vertexBuffer.size;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	indexBufferView.BufferLocation = indexBuffer.resourceBuffer->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = indexBuffer.size;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	Rendering::commandQueue->WaitForAllCommands();
}

void Mesh::Draw(CommandRecorder* recorder)
{
	recorder->list->IASetVertexBuffers(0, 1, &vertexBufferView);
	recorder->list->IASetIndexBuffer(&indexBufferView);
	recorder->list->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);
}
