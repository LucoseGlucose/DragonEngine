#pragma once

#include <assimp/postprocess.h>

#include "CommandRecorder.h"
#include "Buffer.h"

struct VertexData
{
	Vector3 position;
	Vector3 normal;
	Vector2 uv;
	Vector3 tangent;
	Vector3 bitangent;
};

class Mesh
{
public:
	Mesh(std::filesystem::path path);

	std::vector<VertexData> vertices = std::vector<VertexData>();
	std::vector<UINT32> indices = std::vector<UINT32>();

	Buffer vertexBuffer;
	Buffer vertexUploadBuffer;
	Buffer indexBuffer;
	Buffer indexUploadBuffer;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	BoundingBox aabb;

	void UploadMeshData();
	void Draw(CommandRecorder* recorder);

	static const int importFlags = aiProcess_ConvertToLeftHanded | aiProcess_JoinIdenticalVertices
		| aiProcess_OptimizeMeshes | aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenBoundingBoxes;
};