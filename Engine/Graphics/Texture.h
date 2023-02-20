#pragma once



class Texture
{
public:
	Texture();

	ComPtr<ID3D12Resource> textureBuffer;

	D3D12_RESOURCE_DESC textureDesc;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
};