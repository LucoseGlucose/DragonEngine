#pragma once

#include "Mesh.h"
#include "CommandQueue.h"
#include "Material.h"
#include "PresentationBuffer.h"
#include "CommandRecorder.h"
#include "Framebuffer.h"
#include "CameraComponent.h"
#include "RendererComponent.h"
#include "LightComponent.h"
#include "RenderPass.h"
#include "SceneRenderPass.h"
#include "ResolveRenderPass.h"
#include "ProcessRenderPass.h"
#include "Utils.h"

class Rendering
{
public:

	STATIC(ComPtr<IDXGIFactory4> factory);
	STATIC(ComPtr<IDXGIAdapter1> adapter);
	STATIC(ComPtr<ID3D12Device4> device);

	STATIC(CommandQueue* commandQueue);
	STATIC(PresentationBuffer* presentationBuffer);

	STATIC(std::queue<CommandRecorder*>* cmdRecorders);
	STATIC(std::mutex recorderMutex);

	STATIC(D3D12_VIEWPORT viewport);
	STATIC(D3D12_RECT scissorRect);

	STATIC(std::vector<RenderPass*> renderPasses);

	STATIC(Mesh* quadMesh);
	STATIC(Material* missingMaterial);

	STATIC(RendererComponent* outputObj);
	STATIC(CameraComponent* outputCam);

	STATIC(SceneRenderPass* scenePass);
	STATIC(ResolveRenderPass* resolvePass);
	STATIC(ProcessRenderPass* tonemapPass);
	STATIC(ProcessRenderPass* gammaPass);

	static inline std::array<UINT64, Settings::numPresentationFrames> fenceValues{};

	static void WaitForNextFrame();

	static CommandRecorder* GetRecorder();
	static void RecycleRecorder(CommandRecorder* recorder);

	static void Init();
	static void Render();
	static void Cleanup();

	static void Resize(Vector2 newSize);

	static void SetViewportSize(Vector2 size);
	static void ResetViewportSize();

	static D3D12_SAMPLER_DESC GetDefaultSampler();
	static D3D12_SAMPLER_DESC GetBRDFSampler();

	static UINT32 GetMipCount(UINT32 width, UINT32 height);

	static DXGI_FORMAT GetSRGBFormat(DXGI_FORMAT linear);
	static DXGI_FORMAT GetLinearFormat(DXGI_FORMAT srgb);
	static bool IsFormatSRGB(DXGI_FORMAT format);

	static UINT8 GetColorMaskForIndex(UINT8 index);
	static DXGI_FORMAT GetResolveFormatForDepth(DXGI_FORMAT format);
	static void RecordBarriers(CommandRecorder* recorder, std::initializer_list<CD3DX12_RESOURCE_BARRIER> barriers);

	static CD3DX12_BLEND_DESC GetDefaultBlendState();
	static CD3DX12_RASTERIZER_DESC GetDefaultRasterizerState();
	static CD3DX12_DEPTH_STENCIL_DESC GetDefaultDepthStencilState();
	static DXGI_SAMPLE_DESC GetDefaultSampleDesc();
};