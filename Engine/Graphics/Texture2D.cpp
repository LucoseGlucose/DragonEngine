#include "stdafx.h"
#include "Texture2D.h"

#include "Rendering.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture2D::Texture2D(void* data, size_t dataSize, XMUINT2 size, DXGI_FORMAT format, uint32_t bytesPerPixel) : Texture(), size(size)
{
	CommandRecorder* recorder = Rendering::GetRecorder();
	recorder->StartRecording();

	textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, size.x, size.y);
	CD3DX12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&textureBuffer)));
	NAME_D3D_OBJECT(textureBuffer);

	UINT64 textureUploadSize;
	Rendering::device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadSize);

	CD3DX12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(textureUploadSize);

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc,
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&uploadBuffer)));
	NAME_D3D_OBJECT(uploadBuffer);

	D3D12_SUBRESOURCE_DATA subResourceData{};
	subResourceData.pData = data;
	subResourceData.RowPitch = bytesPerPixel * size.x;
	subResourceData.SlicePitch = bytesPerPixel * size.x * size.y;

	UpdateSubresources(recorder->list.Get(), textureBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

	CD3DX12_RESOURCE_BARRIER shaderResourceTransition = CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	recorder->list->ResourceBarrier(1, &shaderResourceTransition);

	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	recorder->Execute();
	Rendering::commandQueue->WaitForAllCommands();
	Rendering::RecycleRecorder(recorder);
}

Texture2D* Texture2D::Import(const std::filesystem::path& file, bool sRGB)
{
	int width, height, numComponents;
	BYTE* imageData = stbi_load(file.string().c_str(), &width, &height, &numComponents, 4);

	return new Texture2D(imageData, sizeof(imageData), XMUINT2(width, height),
		sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM, 4);
}