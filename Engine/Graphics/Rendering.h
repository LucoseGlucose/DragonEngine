#pragma once

#include "Mesh.h"
#include "CommandQueue.h"
#include "Material.h"
#include "PresentationBuffer.h"
#include "CommandRecorder.h"
#include "Framebuffer.h"
#include "CameraComponent.h"
#include "RendererComponent.h"

class Rendering
{
public:
	static inline ComPtr<IDXGIFactory4> factory{};
	static inline ComPtr<IDXGIAdapter1> adapter{};
	static inline ComPtr<ID3D12Device4> device{};

	static inline CommandQueue* commandQueue{};
	static inline PresentationBuffer* presentationBuffer{};
	static inline Framebuffer* sceneFB{};
	static inline Framebuffer* postFB{};

	static inline std::queue<CommandRecorder*> cmdRecorders{};

	static inline D3D12_VIEWPORT viewport{};
	static inline D3D12_RECT scissorRect{};

	static inline Mesh* quadMesh{};
	static inline RendererComponent* outputObj{};

	static inline CameraComponent* outputCam{};

	static inline UINT64 fenceValues[2]{};

	static void WaitForNextFrame();

	static CommandRecorder* GetRecorder();
	static void RecycleRecorder(CommandRecorder* recorder);

	static void Init();
	static void Render();
	static void Cleanup();

	static void Resize(XMUINT2 newSize);
};