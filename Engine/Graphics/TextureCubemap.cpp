#include "stdafx.h"
#include "TextureCubemap.h"

#include "stb_image.h"
#include "Rendering.h"
#include "Texture2D.h"
#include "SkyboxObject.h"

TextureCubemap::TextureCubemap(std::array<void*, 6> data, XMUINT2 size, DXGI_FORMAT format, uint32_t bytesPerPixel)
	: Texture(), size(size)
{
	CommandRecorder* recorder = Rendering::GetRecorder();
	recorder->StartRecording();

	textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, size.x, size.y, 6, 1);
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

	if (data[0] == nullptr)
	{
		recorder->Execute();
		Rendering::commandQueue->WaitForAllCommands();
	}
	else
	{
		for (size_t i = 0; i < 6; i++)
		{
			if (i != 0) recorder->StartRecording();

			D3D12_SUBRESOURCE_DATA subResourceData{};
			subResourceData.pData = data[i];
			subResourceData.RowPitch = bytesPerPixel * size.x;
			subResourceData.SlicePitch = bytesPerPixel * size.x * size.y;

			UpdateSubresources(recorder->list.Get(), textureBuffer.Get(), uploadBuffer.Get(), 0, i, 1, &subResourceData);

			recorder->Execute();
			Rendering::commandQueue->WaitForAllCommands();
		}
	}

	recorder->StartRecording();

	CD3DX12_RESOURCE_BARRIER shaderResourceTransition = CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	recorder->list->ResourceBarrier(1, &shaderResourceTransition);

	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;

	recorder->Execute();
	Rendering::commandQueue->WaitForAllCommands();

	Rendering::RecycleRecorder(recorder);
}

TextureCubemap* TextureCubemap::Import(const std::array<std::filesystem::path, 6>& files, bool sRGB)
{
	std::array<void*, 6> dataPtrs{};
	int width, height, numComponents;

	for (size_t i = 0; i < 6; i++)
	{
		BYTE* imageData = stbi_load(files[i].string().c_str(), &width, &height, &numComponents, 4);
		dataPtrs[i] = imageData;
	}

	TextureCubemap* tex = new TextureCubemap(dataPtrs, XMUINT2(width, height),
		sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM, 4);

	for (size_t i = 0; i < 6; i++)
	{
		stbi_image_free(dataPtrs[i]);
	}

	return tex;
}

TextureCubemap* TextureCubemap::ImportHDR(const std::filesystem::path& file)
{
	Texture2D* equirect = Texture2D::ImportHDR(file);

	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(equirect->format, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	Material* material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("EquirectToCubemapVertex.cso"),
		Utils::GetPathFromExe("EquirectToCubemapPixel.cso"), 1, equirect->format));

	XMUINT2 size = XMUINT2(equirect->size.x / 2, equirect->size.y);
	TextureCubemap* cubemap = new TextureCubemap(std::array<void*, 6>{}, size, DXGI_FORMAT_R32G32B32A32_FLOAT, 16);

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

	material->SetTexture("t_texture", equirect);

	Rendering::SetViewportSize(size);
	std::array<Framebuffer*, 6> fbs{};

	CommandRecorder* recorder = Rendering::GetRecorder();

	for (size_t i = 0; i < 6; i++)
	{
		recorder->StartRecording();

		Framebuffer* fb = new Framebuffer(size, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 1);
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
		Rendering::commandQueue->WaitForAllCommands();
	}

	for (size_t i = 0; i < 6; i++)
	{
		delete fbs[i];
	}

	Rendering::ResetViewportSize();
	Rendering::RecycleRecorder(recorder);

	delete equirect;
	return cubemap;
}

TextureCubemap* TextureCubemap::ComputeDiffuseIrradiance(TextureCubemap* skybox, XMUINT2 size)
{
	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(skybox->srvDesc.Format, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	Material* material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("EquirectToCubemapVertex.cso"),
		Utils::GetPathFromExe("IrradiancePixel.cso"), 1, skybox->srvDesc.Format));

	TextureCubemap* cubemap = new TextureCubemap(std::array<void*, 6>{}, size, DXGI_FORMAT_R32G32B32A32_FLOAT, 16);

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

	material->SetTexture("t_skybox", skybox);

	Rendering::SetViewportSize(size);
	std::array<Framebuffer*, 6> fbs{};

	CommandRecorder* recorder = Rendering::GetRecorder();

	for (size_t i = 0; i < 6; i++)
	{
		recorder->StartRecording();

		Framebuffer* fb = new Framebuffer(size, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 1);
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
		Rendering::commandQueue->WaitForAllCommands();
	}

	for (size_t i = 0; i < 6; i++)
	{
		delete fbs[i];
	}

	Rendering::ResetViewportSize();
	Rendering::RecycleRecorder(recorder);

	return cubemap;
}
