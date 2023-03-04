#include "stdafx.h"
#include "TextureCubemap.h"

#include "stb_image.h"
#include "Rendering.h"
#include "Texture2D.h"
#include "SkyboxObject.h"

TextureCubemap::TextureCubemap(std::array<void*, 6> data, XMUINT2 size, DXGI_FORMAT format, uint32_t bytesPerPixel, uint32_t mipCount)
	: Texture(), size(size), mipCount(mipCount), format(format)
{
	CommandRecorder* recorder = Rendering::GetRecorder();
	recorder->StartRecording();

	textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, size.x, size.y, 6, mipCount);
	CD3DX12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&textureBuffer)));
	NAME_D3D_OBJECT(textureBuffer);

	UINT64 textureUploadSize;
	Rendering::device->GetCopyableFootprints(&textureDesc, 0, 6, 0, nullptr, nullptr, nullptr, &textureUploadSize);

	CD3DX12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(textureUploadSize);

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc,
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&uploadBuffer)));
	NAME_D3D_OBJECT(uploadBuffer);

	if (data[0] != nullptr)
	{
		recorder->Execute();
		Rendering::commandQueue->WaitForAllCommands();

		for (size_t i = 0; i < 6; i++)
		{
			CommandRecorder* localRecorder = Rendering::GetRecorder();
			localRecorder->StartRecording();

			D3D12_SUBRESOURCE_DATA subResourceData{};
			subResourceData.pData = data[i];
			subResourceData.RowPitch = bytesPerPixel * size.x;
			subResourceData.SlicePitch = bytesPerPixel * size.x * size.y;

			UpdateSubresources(localRecorder->list.Get(), textureBuffer.Get(), uploadBuffer.Get(), 0,
				D3D12CalcSubresource(0, i, 0, mipCount, 6), 1, &subResourceData);

			localRecorder->Execute();
			Rendering::RecycleRecorder(localRecorder);
			Rendering::commandQueue->WaitForAllCommands();
		}

		recorder->StartRecording();
	}

	CD3DX12_RESOURCE_BARRIER shaderResourceTransition = CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	recorder->list->ResourceBarrier(1, &shaderResourceTransition);

	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = mipCount;

	recorder->Execute();
	Rendering::RecycleRecorder(recorder);

	if (mipCount > 1 && data[0] != nullptr) GenerateMipMaps();
}

void TextureCubemap::GenerateMipMaps()
{
	DXGI_FORMAT linearFormat = Utils::GetLinearFormat(format);

	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(linearFormat, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	Material* downsampleMat = new Material(ShaderProgram::Create(Utils::GetPathFromExe("CubemapVertex.cso"),
		Utils::GetPathFromExe("MipmapCubePixel.cso"), 1, linearFormat));

	srvDesc.Format = linearFormat;

	downsampleMat->SetTexture("t_texture", this);

	XMMATRIX projection = XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.f), 1.f, .1f, 10.f);
	XMFLOAT3 zero = XMFLOAT3(0.f, 0.f, 0.f);
	XMVECTOR zeroVec = DirectX::XMLoadFloat3(&zero);

	XMFLOAT4X4* matrices = new XMFLOAT4X4[6];
	DirectX::XMStoreFloat4x4(&matrices[0], XMMatrixTranspose(XMMatrixLookToLH(zeroVec, g_XMIdentityR0.v, g_XMIdentityR1.v) * projection));
	DirectX::XMStoreFloat4x4(&matrices[1], XMMatrixTranspose(XMMatrixLookToLH(zeroVec, g_XMNegIdentityR0.v, g_XMIdentityR1.v) * projection));
	DirectX::XMStoreFloat4x4(&matrices[2], XMMatrixTranspose(XMMatrixLookToLH(zeroVec, g_XMIdentityR1.v, g_XMNegIdentityR2.v) * projection));
	DirectX::XMStoreFloat4x4(&matrices[3], XMMatrixTranspose(XMMatrixLookToLH(zeroVec, g_XMNegIdentityR1.v, g_XMIdentityR2.v) * projection));
	DirectX::XMStoreFloat4x4(&matrices[4], XMMatrixTranspose(XMMatrixLookToLH(zeroVec, g_XMIdentityR2.v, g_XMIdentityR1.v) * projection));
	DirectX::XMStoreFloat4x4(&matrices[5], XMMatrixTranspose(XMMatrixLookToLH(zeroVec, g_XMNegIdentityR2.v, g_XMIdentityR1.v) * projection));

	for (float mip = 1; mip < mipCount; mip++)
	{
		std::array<Framebuffer*, 6> fbs{};

		XMUINT2 mipSize = XMUINT2(size.x * std::powf(.5f, mip), size.y * std::powf(.5f, mip));
		Rendering::SetViewportSize(mipSize);

		for (size_t i = 0; i < 6; i++)
		{
			CommandRecorder* recorder = Rendering::GetRecorder();
			recorder->StartRecording();

			Framebuffer* fb = new Framebuffer(mipSize, linearFormat, DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 1);
			fbs[i] = fb;

			fb->Setup(recorder);

			downsampleMat->SetParameter("p_mvpMat", &matrices[i], sizeof(XMFLOAT4X4));
			downsampleMat->SetParameter("p_currentMip", &mip, sizeof(float));

			uint32_t subresource = D3D12CalcSubresource(mip, i, 0, mipCount, 6);

			downsampleMat->Bind(recorder);
			SkyboxObject::skyboxMesh->Draw(recorder);

			CD3DX12_RESOURCE_BARRIER copySrcTransition = CD3DX12_RESOURCE_BARRIER::Transition(fb->colorTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

			CD3DX12_RESOURCE_BARRIER copyDestTransition = CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer.Get(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, subresource);

			CD3DX12_RESOURCE_BARRIER copyBarriers[2] = { copySrcTransition, copyDestTransition };
			recorder->list->ResourceBarrier(2, copyBarriers);

			D3D12_TEXTURE_COPY_LOCATION dest{};
			dest.pResource = textureBuffer.Get();
			dest.SubresourceIndex = subresource;
			dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			D3D12_TEXTURE_COPY_LOCATION src{};
			src.pResource = fb->colorTexture->textureBuffer.Get();
			src.SubresourceIndex = 0;
			src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			recorder->list->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);

			CD3DX12_RESOURCE_BARRIER srvTransition = CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, subresource);

			recorder->list->ResourceBarrier(1, &srvTransition);

			recorder->Execute();
			Rendering::RecycleRecorder(recorder);

			Rendering::commandQueue->WaitForAllCommands();
		}

		for (size_t i = 0; i < 6; i++)
		{
			delete fbs[i];
		}
	}

	Rendering::ResetViewportSize();

	srvDesc.Format = format;
}

XMFLOAT4X4* TextureCubemap::GetCubemapMatrices()
{
	XMMATRIX projection = XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.f), 1.f, .1f, 10.f);
	XMFLOAT3 zero = XMFLOAT3(0.f, 0.f, 0.f);
	XMVECTOR zeroVec = DirectX::XMLoadFloat3(&zero);

	XMFLOAT4X4* matrices = new XMFLOAT4X4[6];
	DirectX::XMStoreFloat4x4(&matrices[0], XMMatrixTranspose(XMMatrixLookToLH(zeroVec, g_XMIdentityR0.v, g_XMIdentityR1.v) * projection));
	DirectX::XMStoreFloat4x4(&matrices[1], XMMatrixTranspose(XMMatrixLookToLH(zeroVec, g_XMNegIdentityR0.v, g_XMIdentityR1.v) * projection));
	DirectX::XMStoreFloat4x4(&matrices[2], XMMatrixTranspose(XMMatrixLookToLH(zeroVec, g_XMIdentityR1.v, g_XMNegIdentityR2.v) * projection));
	DirectX::XMStoreFloat4x4(&matrices[3], XMMatrixTranspose(XMMatrixLookToLH(zeroVec, g_XMNegIdentityR1.v, g_XMIdentityR2.v) * projection));
	DirectX::XMStoreFloat4x4(&matrices[4], XMMatrixTranspose(XMMatrixLookToLH(zeroVec, g_XMIdentityR2.v, g_XMIdentityR1.v) * projection));
	DirectX::XMStoreFloat4x4(&matrices[5], XMMatrixTranspose(XMMatrixLookToLH(zeroVec, g_XMNegIdentityR2.v, g_XMIdentityR1.v) * projection));

	return matrices;
}

TextureCubemap* TextureCubemap::Import(const std::array<std::filesystem::path, 6>& files, bool sRGB, bool generateMips)
{
	std::array<void*, 6> dataPtrs{};
	int width, height, numComponents;

	for (size_t i = 0; i < 6; i++)
	{
		BYTE* imageData = stbi_load(files[i].string().c_str(), &width, &height, &numComponents, 4);
		dataPtrs[i] = imageData;
	}

	uint32_t mipCount = generateMips ? Utils::GetMipCount(width, height) : 1;
	TextureCubemap* tex = new TextureCubemap(dataPtrs, XMUINT2(width, height),
		sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM, 4, mipCount);

	for (size_t i = 0; i < 6; i++)
	{
		stbi_image_free(dataPtrs[i]);
	}

	return tex;
}

TextureCubemap* TextureCubemap::ImportHDR(const std::filesystem::path& file, bool generateMips)
{
	Texture2D* equirect = Texture2D::ImportHDR(file);

	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(equirect->format, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	Material* material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("CubemapVertex.cso"),
		Utils::GetPathFromExe("EquirectToCubemapPixel.cso"), 1, equirect->format));

	XMUINT2 size = XMUINT2(equirect->size.x / 2, equirect->size.y);
	uint32_t mipCount = generateMips ? Utils::GetMipCount(size.x, size.y) : 1;
	TextureCubemap* cubemap = new TextureCubemap(std::array<void*, 6>{}, size, DXGI_FORMAT_R32G32B32A32_FLOAT, 16, mipCount);

	XMFLOAT4X4* matrices = GetCubemapMatrices();
	material->SetTexture("t_texture", equirect);

	Rendering::SetViewportSize(size);
	std::array<Framebuffer*, 6> fbs{};

	for (size_t i = 0; i < 6; i++)
	{
		CommandRecorder* recorder = Rendering::GetRecorder();
		recorder->StartRecording();

		Framebuffer* fb = new Framebuffer(size, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 1);
		fbs[i] = fb;

		fb->Setup(recorder);
		material->SetParameter("p_mvpMat", &matrices[i], sizeof(XMFLOAT4X4));

		material->Bind(recorder);
		SkyboxObject::skyboxMesh->Draw(recorder);

		uint32_t subresource = D3D12CalcSubresource(0, i, 0, mipCount, 6);

		CD3DX12_RESOURCE_BARRIER copySrcTransition = CD3DX12_RESOURCE_BARRIER::Transition(fb->colorTexture->textureBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

		CD3DX12_RESOURCE_BARRIER copyDestTransition = CD3DX12_RESOURCE_BARRIER::Transition(cubemap->textureBuffer.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, subresource);

		CD3DX12_RESOURCE_BARRIER copyBarriers[2] = { copySrcTransition, copyDestTransition };
		recorder->list->ResourceBarrier(2, copyBarriers);

		D3D12_TEXTURE_COPY_LOCATION dest{};
		dest.pResource = cubemap->textureBuffer.Get();
		dest.SubresourceIndex = subresource;
		dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION src{};
		src.pResource = fb->colorTexture->textureBuffer.Get();
		src.SubresourceIndex = 0;
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		recorder->list->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);

		CD3DX12_RESOURCE_BARRIER srvTransition = CD3DX12_RESOURCE_BARRIER::Transition(cubemap->textureBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, subresource);

		recorder->list->ResourceBarrier(1, &srvTransition);

		recorder->Execute();
		Rendering::RecycleRecorder(recorder);
		Rendering::commandQueue->WaitForAllCommands();
	}

	Rendering::ResetViewportSize();

	for (size_t i = 0; i < 6; i++)
	{
		delete fbs[i];
	}
	delete equirect;

	if (mipCount > 1) cubemap->GenerateMipMaps();

	return cubemap;
}

TextureCubemap* TextureCubemap::ComputeDiffuseIrradiance(TextureCubemap* skybox, XMUINT2 size)
{
	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	Material* material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("CubemapVertex.cso"),
		Utils::GetPathFromExe("IrradiancePixel.cso"), 1, DXGI_FORMAT_R16G16B16A16_FLOAT));

	TextureCubemap* cubemap = new TextureCubemap(std::array<void*, 6>{}, size, DXGI_FORMAT_R16G16B16A16_FLOAT, 8, 1);
	XMFLOAT4X4* matrices = GetCubemapMatrices();
	material->SetTexture("t_skybox", skybox);

	Rendering::SetViewportSize(size);
	std::array<Framebuffer*, 6> fbs{};

	for (size_t i = 0; i < 6; i++)
	{
		CommandRecorder* recorder = Rendering::GetRecorder();
		recorder->StartRecording();

		Framebuffer* fb = new Framebuffer(size, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 1);
		fbs[i] = fb;

		fb->Setup(recorder);
		material->SetParameter("p_mvpMat", &matrices[i], sizeof(XMFLOAT4X4));

		material->Bind(recorder);
		SkyboxObject::skyboxMesh->Draw(recorder);

		CD3DX12_RESOURCE_BARRIER copySrcTransition = CD3DX12_RESOURCE_BARRIER::Transition(fb->colorTexture->textureBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

		CD3DX12_RESOURCE_BARRIER copyDestTransition = CD3DX12_RESOURCE_BARRIER::Transition(cubemap->textureBuffer.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, i);

		CD3DX12_RESOURCE_BARRIER copyBarriers[2] = { copySrcTransition, copyDestTransition };
		recorder->list->ResourceBarrier(2, copyBarriers);

		D3D12_TEXTURE_COPY_LOCATION dest{};
		dest.pResource = cubemap->textureBuffer.Get();
		dest.SubresourceIndex = i;
		dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION src{};
		src.pResource = fb->colorTexture->textureBuffer.Get();
		src.SubresourceIndex = 0;
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		recorder->list->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);

		CD3DX12_RESOURCE_BARRIER srvTransition = CD3DX12_RESOURCE_BARRIER::Transition(cubemap->textureBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, i);

		recorder->list->ResourceBarrier(1, &srvTransition);

		recorder->Execute();
		Rendering::RecycleRecorder(recorder);
		Rendering::commandQueue->WaitForAllCommands();
	}

	Rendering::ResetViewportSize();

	for (size_t i = 0; i < 6; i++)
	{
		delete fbs[i];
	}

	return cubemap;
}

TextureCubemap* TextureCubemap::ComputeAmbientSpecular(TextureCubemap* skybox, XMUINT2 size, uint32_t mipCount)
{
	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	Material* material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("CubemapVertex.cso"),
		Utils::GetPathFromExe("AmbientSpecularPixel.cso"), 1, DXGI_FORMAT_R16G16B16A16_FLOAT));

	TextureCubemap* cubemap = new TextureCubemap(std::array<void*, 6>{}, size, DXGI_FORMAT_R16G16B16A16_FLOAT, 8, mipCount);
	XMFLOAT4X4* matrices = GetCubemapMatrices();
	material->SetTexture("t_skybox", skybox);

	for (float mip = 0; mip < mipCount; mip++)
	{
		std::array<Framebuffer*, 6> fbs{};

		XMUINT2 mipSize = XMUINT2(size.x * std::powf(.5f, mip), size.y * std::powf(.5f, mip));
		Rendering::SetViewportSize(mipSize);

		for (size_t i = 0; i < 6; i++)
		{
			CommandRecorder* recorder = Rendering::GetRecorder();
			recorder->StartRecording();

			Framebuffer* fb = new Framebuffer(mipSize, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 1);
			fbs[i] = fb;

			fb->Setup(recorder);

			material->SetParameter("p_mvpMat", &matrices[i], sizeof(XMFLOAT4X4));

			float resolution = skybox->size.x;
			material->SetParameter("p_resolution", &resolution, sizeof(float));

			float roughness = (1.f / (mipCount - 1)) * mip;
			material->SetParameter("p_roughness", &roughness, sizeof(float));

			uint32_t subresource = D3D12CalcSubresource(mip, i, 0, mipCount, 6);

			material->Bind(recorder);
			SkyboxObject::skyboxMesh->Draw(recorder);

			CD3DX12_RESOURCE_BARRIER copySrcTransition = CD3DX12_RESOURCE_BARRIER::Transition(fb->colorTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

			CD3DX12_RESOURCE_BARRIER copyDestTransition = CD3DX12_RESOURCE_BARRIER::Transition(cubemap->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, subresource);

			CD3DX12_RESOURCE_BARRIER copyBarriers[2] = { copySrcTransition, copyDestTransition };
			recorder->list->ResourceBarrier(2, copyBarriers);

			D3D12_TEXTURE_COPY_LOCATION dest{};
			dest.pResource = cubemap->textureBuffer.Get();
			dest.SubresourceIndex = subresource;
			dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			D3D12_TEXTURE_COPY_LOCATION src{};
			src.pResource = fb->colorTexture->textureBuffer.Get();
			src.SubresourceIndex = 0;
			src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			recorder->list->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);

			CD3DX12_RESOURCE_BARRIER srvTransition = CD3DX12_RESOURCE_BARRIER::Transition(cubemap->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, subresource);

			recorder->list->ResourceBarrier(1, &srvTransition);

			recorder->Execute();
			Rendering::RecycleRecorder(recorder);
			Rendering::commandQueue->WaitForAllCommands();
		}

		Rendering::ResetViewportSize();

		for (size_t i = 0; i < 6; i++)
		{
			delete fbs[i];
		}
	}

	return cubemap;
}
