#include "stdafx.h"
#include "Texture2D.h"

#include "Rendering.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture2D::Texture2D(void* data, XMUINT2 size, uint32_t bytesPerPixel, uint32_t mipCount, DXGI_FORMAT format)
	: Texture(), size(size), mipCount(mipCount), format(format)
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

	if (data != nullptr)
	{
		D3D12_SUBRESOURCE_DATA subResourceData{};
		subResourceData.pData = data;
		subResourceData.RowPitch = bytesPerPixel * size.x;
		subResourceData.SlicePitch = bytesPerPixel * size.x * size.y;

		UpdateSubresources(recorder->list.Get(), textureBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	}

	CD3DX12_RESOURCE_BARRIER shaderResourceTransition = CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	recorder->list->ResourceBarrier(1, &shaderResourceTransition);

	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = mipCount;

	if (mipCount > 1 && data != nullptr) GenerateMipMaps(recorder);
	else recorder->Execute();

	Rendering::RecycleRecorder(recorder);
}

Texture2D* Texture2D::Import(const std::filesystem::path& file, bool sRGB, bool generateMips)
{
	int width, height, numComponents;
	BYTE* imageData = stbi_load(file.string().c_str(), &width, &height, &numComponents, 4);

	uint32_t mipCount = generateMips ? Utils::GetMipCount(width, height) : 1;
	DXGI_FORMAT format = sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

	Texture2D* tex = new Texture2D(imageData, XMUINT2(width, height), 4, mipCount, format);

	stbi_image_free(imageData);
	return tex;
}

Texture2D* Texture2D::ImportHDR(const std::filesystem::path& file)
{
	int width, height, numComponents;
	XMFLOAT3* imageData = (XMFLOAT3*)stbi_loadf(file.string().c_str(), &width, &height, &numComponents, 0);

	XMFLOAT4* rgbaData = new XMFLOAT4[width * height];
	for (size_t x = 0; x < width; x++)
	{
		for (size_t y = 0; y < height; y++)
		{
			XMFLOAT3 color = imageData[x + y * width];
			rgbaData[x + y * width] = XMFLOAT4(color.x, color.y, color.z, 1.0f);
		}
	}

	Texture2D* tex = new Texture2D(rgbaData, XMUINT2(width, height), 16, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);

	delete[] rgbaData;
	stbi_image_free(imageData);

	return tex;
}

void Texture2D::GenerateMipMaps(CommandRecorder* recorder)
{
	DXGI_FORMAT linearFormat = Utils::GetLinearFormat(format);

	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(linearFormat, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	Material* downsampleMat = new Material(ShaderProgram::Create(Utils::GetPathFromExe("MipmapVertex.cso"),
		Utils::GetPathFromExe("MipmapPixel.cso"), 1, linearFormat));

	srvDesc.Format = linearFormat;

	downsampleMat->SetTexture("t_texture", this);

	Framebuffer** fbs = new Framebuffer*[mipCount - 1];

	for (uint32_t mip = 1; mip < mipCount; mip++)
	{
		if (mip != 1) recorder->StartRecording();

		XMUINT2 mipSize = XMUINT2(size.x * std::powf(.5f, mip), size.y * std::powf(.5f, mip));
		Rendering::SetViewportSize(mipSize);

		Framebuffer* fb = new Framebuffer(XMUINT2(Rendering::viewport.Width, Rendering::viewport.Height),
			linearFormat, DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 1);
		fbs[mip - 1] = fb;

		fb->Setup(recorder, true);
		downsampleMat->SetParameter("p_currentMip", &mip, sizeof(uint32_t));

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

	srvDesc.Format = format;
	Rendering::ResetViewportSize();
}

Texture2D* Texture2D::GetWhiteTexture()
{
	if (whiteTexture != nullptr) return whiteTexture;

	uint32_t white = 0xffffffff;
	whiteTexture = new Texture2D(&white, XMUINT2(1, 1), 4, 1, DXGI_FORMAT_R8G8B8A8_UNORM);

	return whiteTexture;
}

Texture2D* Texture2D::GetNormalTexture()
{
	if (normalTexture != nullptr) return normalTexture;

	BYTE col[4] = { 127, 127, 255, 255 };
	normalTexture = new Texture2D(col, XMUINT2(1, 1), 4, 1, DXGI_FORMAT_R8G8B8A8_UNORM);

	return normalTexture;
}

Texture2D* Texture2D::GetBRDFTexture()
{
	if (brdfTexture != nullptr) return brdfTexture;

	brdfTexture = Import(Utils::GetPathFromProject("Images/BRDF LUT.png"), false, false);

	return brdfTexture;
}
