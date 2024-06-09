#pragma once

#include "GraphicsResource.h"

class Texture : public GraphicsResource
{
public:

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
};