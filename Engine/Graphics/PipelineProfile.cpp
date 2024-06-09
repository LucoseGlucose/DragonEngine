#include "stdafx.h"
#include "PipelineProfile.h"

#include "Rendering.h"

PipelineProfile::PipelineProfile()
	:
	blendState(Rendering::GetDefaultBlendState()),
	rasterizerState(Rendering::GetDefaultRasterizerState()),
	depthStencilState(Rendering::GetDefaultDepthStencilState()),
	topologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE),
	rtvFormats{  },
	dsvFormat(),
	sampleDesc(Rendering::GetDefaultSampleDesc())
{

}

bool PipelineProfile::operator<(const PipelineProfile& r) const
{
	return false;
}
