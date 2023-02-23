#include "stdafx.h"
#include "Texture2D.h"

#include "Rendering.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture2D::Texture2D(void* data, size_t dataSize, XMUINT2 size, DXGI_FORMAT format, uint32_t bytesPerPixel, uint32_t mipCount)
	: Texture(), size(size), mipCount(mipCount)
{
	CommandRecorder* recorder = Rendering::GetRecorder();
	recorder->StartRecording();

	textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, size.x, size.y, 1, mipCount);
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
	srvDesc.Texture2D.MipLevels = mipCount;

	if (mipCount > 1) GenerateMipMaps(recorder);
	else
	{
		recorder->Execute();
		Rendering::commandQueue->WaitForAllCommands();
	}

	Rendering::RecycleRecorder(recorder);
}

Texture2D* Texture2D::Import(const std::filesystem::path& file, bool sRGB, bool generateMips)
{
	int width, height, numComponents;
	BYTE* imageData = stbi_load(file.string().c_str(), &width, &height, &numComponents, 4);

	uint32_t mipCount = generateMips ? Utils::GetMipCount(width, height) : 1;

	return new Texture2D(imageData, sizeof(imageData), XMUINT2(width, height),
		sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM, 4, mipCount);
}

void Texture2D::GenerateMipMaps(CommandRecorder* recorder)
{
	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	D3D12_VIEWPORT startViewport = Rendering::viewport;
	Material* downsampleMat = new Material(ShaderProgram::Create(Utils::GetPathFromExe("MipmapVertex.cso"),
		Utils::GetPathFromExe("MipmapPixel.cso"), 1, DXGI_FORMAT_R8G8B8A8_UNORM));

	downsampleMat->SetSampler("s", Utils::GetDefaultSampler());
	downsampleMat->SetTexture("t", this);

	Rendering::viewport.Width = size.x;
	Rendering::viewport.Height = size.y;

	Framebuffer** fbs = new Framebuffer*[mipCount - 1];

	for (uint32_t mip = 1; mip < mipCount; mip++)
	{
		if (mip != 1) recorder->StartRecording();

		Rendering::viewport.Width = Rendering::viewport.Width * .5f;
		Rendering::viewport.Height = Rendering::viewport.Height * .5f;

		Framebuffer* fb = new Framebuffer(XMUINT2(Rendering::viewport.Width, Rendering::viewport.Height),
			DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 1);
		fbs[mip - 1] = fb;

		fb->Setup(recorder);
		downsampleMat->SetParameter("currentMip", &mip, sizeof(uint32_t));

		downsampleMat->Bind(recorder);
		Rendering::quadMesh->Draw(recorder);

		CD3DX12_RESOURCE_BARRIER copySrcTransition = CD3DX12_RESOURCE_BARRIER::Transition(fb->colorTexture->textureBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

		CD3DX12_RESOURCE_BARRIER copyDestTransition = CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, mip);

		CD3DX12_RESOURCE_BARRIER copyBarriers[2] = { copySrcTransition, copyDestTransition };
		recorder->list->ResourceBarrier(2, copyBarriers);

		D3D12_TEXTURE_COPY_LOCATION dest{};
		dest.pResource = textureBuffer.Get();
		dest.SubresourceIndex = mip;
		dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION src{};
		src.pResource = fb->colorTexture->textureBuffer.Get();
		src.SubresourceIndex = 0;
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		recorder->list->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);

		CD3DX12_RESOURCE_BARRIER srvTransition = CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, mip);

		recorder->list->ResourceBarrier(1, &srvTransition);

		recorder->Execute();
		Rendering::commandQueue->WaitForAllCommands();
	}

	for (size_t i = 0; i < mipCount - 1; i++)
	{
		delete fbs[i];
	}

	Rendering::viewport = startViewport;
}
