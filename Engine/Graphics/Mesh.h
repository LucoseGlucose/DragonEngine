#pragma once

#include <assimp/postprocess.h>
#include "CommandRecorder.h"

struct VertexData
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
	XMFLOAT3 tangent;
	XMFLOAT3 bitangent;
};

class Mesh
{
public:
	Mesh(std::filesystem::path path);

	std::vector<VertexData> vertices = std::vector<VertexData>();
	std::vector<uint32_t> indices = std::vector<uint32_t>();

	ComPtr<ID3D12Resource> vertexBuffer;
	ComPtr<ID3D12Resource> vertexUploadBuffer;
	ComPtr<ID3D12Resource> indexBuffer;
	ComPtr<ID3D12Resource> indexUploadBuffer;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	void UploadMeshData();
	void Draw(CommandRecorder* recorder);

	static const int importFlags = aiProcess_ConvertToLeftHanded | aiProcess_JoinIdenticalVertices
		| aiProcess_OptimizeMeshes | aiProcess_Triangulate | aiProcess_CalcTangentSpace;
};