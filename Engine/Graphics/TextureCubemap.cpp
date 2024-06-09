#include "stdafx.h"
#include "TextureCubemap.h"

#include "stb_image.h"
#include "Rendering.h"
#include "Texture2D.h"

TextureCubemap::TextureCubemap(std::array<void*, 6> data, Vector2 size, DXGI_FORMAT format, UINT32 bytesPerPixel, UINT32 mipCount)
	: Texture(), size(size), mipCount(mipCount), format(format)
{
	currentState = D3D12_RESOURCE_STATE_COPY_DEST;

	CommandRecorder* recorder = Rendering::GetRecorder();
	recorder->StartRecording();

	resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, size.x, size.y, 6, mipCount);
	CD3DX12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	Utils::ThrowIfFailed(Rendering::device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resourceBuffer)));
	NAME_D3D_OBJECT(resourceBuffer);

	if (data[0] != nullptr)
	{
		UINT64 textureUploadSize;
		Rendering::device->GetCopyableFootprints(&resourceDesc, 0, 6, 0, nullptr, nullptr, nullptr, &textureUploadSize);

		Buffer uploadBuffer = Buffer(textureUploadSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_COPY_SOURCE);

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

			UpdateSubresources(localRecorder->list.Get(), resourceBuffer.Get(), uploadBuffer.resourceBuffer.Get(), 0,
				D3D12CalcSubresource(0, i, 0, mipCount, 6), 1, &subResourceData);

			localRecorder->Execute();

			Rendering::RecycleRecorder(localRecorder);
			Rendering::commandQueue->WaitForAllCommands();
		}

		recorder->StartRecording();
	}

	Rendering::RecordBarriers(recorder, { TransitionToState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) });

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
	DXGI_FORMAT linearFormat = Rendering::GetLinearFormat(format);

	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(linearFormat, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	Material* downsampleMat = new Material(ShaderProgram::Create(
		Utils::GetPathFromExe("CubemapV.cso"), Utils::GetPathFromExe("MipmapCubeP.cso")));

	srvDesc.Format = linearFormat;
	downsampleMat->SetTexture("t_texture", this);

	std::array<Matrix, 6> matrices = GetCubemapMatrices();

	for (float mip = 1; mip < mipCount; mip++)
	{
		std::array<Framebuffer*, 6> fbs{};

		Vector2 mipSize = Vector2(size.x * std::powf(.5f, mip), size.y * std::powf(.5f, mip));
		Rendering::SetViewportSize(mipSize);

		Mesh skyboxMesh = Mesh(Utils::GetPathFromProject("Models/Inverted Cube.fbx"));

		for (size_t i = 0; i < 6; i++)
		{
			CommandRecorder* recorder = Rendering::GetRecorder();
			recorder->StartRecording();

			Framebuffer* fb = new Framebuffer({ RenderTextureProfile(mipSize, linearFormat, rtClear, 1) });
			fbs[i] = fb;

			fb->Setup(recorder);

			downsampleMat->SetParameter("p_mvpMat", &matrices[i]);
			downsampleMat->SetParameter("p_currentMip", &mip);

			UINT32 subresource = D3D12CalcSubresource(mip, i, 0, mipCount, 6);

			downsampleMat->Bind(recorder, fb->pipelineProfile);
			skyboxMesh.Draw(recorder);

			CD3DX12_RESOURCE_BARRIER copySrcTransition = fb->colorTextures.front()->TransitionToState(D3D12_RESOURCE_STATE_COPY_SOURCE);

			CD3DX12_RESOURCE_BARRIER copyDestTransition = CD3DX12_RESOURCE_BARRIER::Transition(resourceBuffer.Get(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, subresource);

			Rendering::RecordBarriers(recorder, { copySrcTransition, copyDestTransition });

			D3D12_TEXTURE_COPY_LOCATION src{};
			src.pResource = fb->colorTextures.front()->resourceBuffer.Get();
			src.SubresourceIndex = 0;
			src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			D3D12_TEXTURE_COPY_LOCATION dest{};
			dest.pResource = resourceBuffer.Get();
			dest.SubresourceIndex = subresource;
			dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			recorder->list->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);

			CD3DX12_RESOURCE_BARRIER srvTransition = CD3DX12_RESOURCE_BARRIER::Transition(resourceBuffer.Get(),
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

std::array<Matrix, 6> TextureCubemap::GetCubemapMatrices()
{
	Matrix projection = Matrix::CreatePerspectiveFieldOfView(DirectX::XMConvertToRadians(90.f), 1.f, .1f, 10.f);

	return std::array<Matrix, 6>
	{
		(Matrix::CreateLookAt(Vector3::Zero, Vector3::UnitX, Vector3::UnitY) * projection).Transpose(),
		(Matrix::CreateLookAt(Vector3::Zero, -Vector3::UnitX, Vector3::UnitY) * projection).Transpose(),
		(Matrix::CreateLookAt(Vector3::Zero, Vector3::UnitY, -Vector3::UnitZ) * projection).Transpose(),
		(Matrix::CreateLookAt(Vector3::Zero, -Vector3::UnitY, Vector3::UnitZ) * projection).Transpose(),
		(Matrix::CreateLookAt(Vector3::Zero, Vector3::UnitZ, Vector3::UnitY) * projection).Transpose(),
		(Matrix::CreateLookAt(Vector3::Zero, -Vector3::UnitZ, Vector3::UnitY) * projection).Transpose(),
	};
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

	UINT32 mipCount = generateMips ? Rendering::GetMipCount(width, height) : 1;
	TextureCubemap* tex = new TextureCubemap(dataPtrs, Vector2(width, height),
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

	Material* material = new Material(ShaderProgram::Create(
		Utils::GetPathFromExe("CubemapV.cso"), Utils::GetPathFromExe("EquirectToCubemapP.cso")));

	Vector2 size = Vector2(equirect->size.x / 2, equirect->size.y);
	UINT32 mipCount = generateMips ? Rendering::GetMipCount(size.x, size.y) : 1;
	TextureCubemap* cubemap = new TextureCubemap(std::array<void*, 6>{}, size, DXGI_FORMAT_R32G32B32A32_FLOAT, 16, mipCount);

	std::array<Matrix, 6> matrices = GetCubemapMatrices();
	material->SetTexture("t_texture", equirect);

	Rendering::SetViewportSize(size);
	std::array<Framebuffer*, 6> fbs{};

	Mesh skyboxMesh = Mesh(Utils::GetPathFromProject("Models/Inverted Cube.fbx"));

	for (size_t i = 0; i < 6; i++)
	{
		CommandRecorder* recorder = Rendering::GetRecorder();
		recorder->StartRecording();

		Framebuffer* fb = new Framebuffer({ RenderTextureProfile(size, DXGI_FORMAT_R32G32B32A32_FLOAT, rtClear, 1) });
		fbs[i] = fb;

		fb->Setup(recorder);
		material->SetParameter("p_mvpMat", &matrices[i]);

		material->Bind(recorder, fb->pipelineProfile);
		skyboxMesh.Draw(recorder);

		UINT32 subresource = D3D12CalcSubresource(0, i, 0, mipCount, 6);

		CD3DX12_RESOURCE_BARRIER copySrcTransition = fb->colorTextures.front()->TransitionToState(D3D12_RESOURCE_STATE_COPY_SOURCE);

		CD3DX12_RESOURCE_BARRIER copyDestTransition = CD3DX12_RESOURCE_BARRIER::Transition(cubemap->resourceBuffer.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, subresource);

		Rendering::RecordBarriers(recorder, { copySrcTransition, copyDestTransition });

		D3D12_TEXTURE_COPY_LOCATION src{};
		src.pResource = fb->colorTextures.front()->resourceBuffer.Get();
		src.SubresourceIndex = 0;
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION dest{};
		dest.pResource = cubemap->resourceBuffer.Get();
		dest.SubresourceIndex = subresource;
		dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		recorder->list->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);

		CD3DX12_RESOURCE_BARRIER srvTransition = CD3DX12_RESOURCE_BARRIER::Transition(cubemap->resourceBuffer.Get(),
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

TextureCubemap* TextureCubemap::ComputeDiffuseIrradiance(TextureCubemap* skybox, Vector2 size)
{
	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	Material* material = new Material(ShaderProgram::Create(
		Utils::GetPathFromExe("CubemapV.cso"), Utils::GetPathFromExe("IrradianceP.cso")));

	TextureCubemap* cubemap = new TextureCubemap(std::array<void*, 6>{}, size, DXGI_FORMAT_R16G16B16A16_FLOAT, 8, 1);
	std::array<Matrix, 6> matrices = GetCubemapMatrices();
	material->SetTexture("t_skybox", skybox);

	Rendering::SetViewportSize(size);
	std::array<Framebuffer*, 6> fbs{};

	Mesh skyboxMesh = Mesh(Utils::GetPathFromProject("Models/Inverted Cube.fbx"));

	for (size_t i = 0; i < 6; i++)
	{
		CommandRecorder* recorder = Rendering::GetRecorder();
		recorder->StartRecording();

		Framebuffer* fb = new Framebuffer({ RenderTextureProfile(size, DXGI_FORMAT_R16G16B16A16_FLOAT, rtClear, 1) });
		fbs[i] = fb;

		fb->Setup(recorder);
		material->SetParameter("p_mvpMat", &matrices[i], sizeof(XMFLOAT4X4));

		material->Bind(recorder, fb->pipelineProfile);
		skyboxMesh.Draw(recorder);

		CD3DX12_RESOURCE_BARRIER copySrcTransition = fb->colorTextures.front()->TransitionToState(D3D12_RESOURCE_STATE_COPY_SOURCE);

		CD3DX12_RESOURCE_BARRIER copyDestTransition = CD3DX12_RESOURCE_BARRIER::Transition(cubemap->resourceBuffer.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, i);

		Rendering::RecordBarriers(recorder, { copySrcTransition, copyDestTransition });

		D3D12_TEXTURE_COPY_LOCATION src{};
		src.pResource = fb->colorTextures.front()->resourceBuffer.Get();
		src.SubresourceIndex = 0;
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION dest{};
		dest.pResource = cubemap->resourceBuffer.Get();
		dest.SubresourceIndex = i;
		dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		recorder->list->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);

		CD3DX12_RESOURCE_BARRIER srvTransition = CD3DX12_RESOURCE_BARRIER::Transition(cubemap->resourceBuffer.Get(),
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

TextureCubemap* TextureCubemap::ComputeAmbientSpecular(TextureCubemap* skybox, Vector2 size, UINT32 mipCount, float sampleCount)
{
	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	Material* material = new Material(ShaderProgram::Create(
		Utils::GetPathFromExe("CubemapV.cso"), Utils::GetPathFromExe("AmbientSpecularP.cso")));

	TextureCubemap* cubemap = new TextureCubemap(std::array<void*, 6>{}, size, DXGI_FORMAT_R16G16B16A16_FLOAT, 8, mipCount);
	std::array<Matrix, 6> matrices = GetCubemapMatrices();

	material->SetTexture("t_skybox", skybox);
	Mesh skyboxMesh = Mesh(Utils::GetPathFromProject("Models/Inverted Cube.fbx"));

	for (float mip = 0; mip < mipCount; mip++)
	{
		std::array<Framebuffer*, 6> fbs{};

		Vector2 mipSize = Vector2(size.x * std::powf(.5f, mip), size.y * std::powf(.5f, mip));
		Rendering::SetViewportSize(mipSize);

		for (size_t i = 0; i < 6; i++)
		{
			CommandRecorder* recorder = Rendering::GetRecorder();
			recorder->StartRecording();

			Framebuffer* fb = new Framebuffer({ RenderTextureProfile(mipSize, DXGI_FORMAT_R16G16B16A16_FLOAT, rtClear, 1) });
			fbs[i] = fb;

			fb->Setup(recorder);

			material->SetParameter("p_sampleCount", sampleCount);
			material->SetParameter("p_mvpMat", &matrices[i]);

			float resolution = skybox->size.x;
			material->SetParameter("p_resolution", &resolution);

			float roughness = (1.f / (mipCount - 1)) * mip;
			material->SetParameter("p_roughness", &roughness);

			UINT32 subresource = D3D12CalcSubresource(mip, i, 0, mipCount, 6);

			material->Bind(recorder, fb->pipelineProfile);
			skyboxMesh.Draw(recorder);

			CD3DX12_RESOURCE_BARRIER copySrcTransition = fb->colorTextures.front()->TransitionToState(D3D12_RESOURCE_STATE_COPY_SOURCE);

			CD3DX12_RESOURCE_BARRIER copyDestTransition = CD3DX12_RESOURCE_BARRIER::Transition(cubemap->resourceBuffer.Get(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, subresource);

			Rendering::RecordBarriers(recorder, { copySrcTransition, copyDestTransition });

			D3D12_TEXTURE_COPY_LOCATION src{};
			src.pResource = fb->colorTextures.front()->resourceBuffer.Get();
			src.SubresourceIndex = 0;
			src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			D3D12_TEXTURE_COPY_LOCATION dest{};
			dest.pResource = cubemap->resourceBuffer.Get();
			dest.SubresourceIndex = subresource;
			dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			recorder->list->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);

			CD3DX12_RESOURCE_BARRIER srvTransition = CD3DX12_RESOURCE_BARRIER::Transition(cubemap->resourceBuffer.Get(),
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
