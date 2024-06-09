#pragma once



struct PipelineProfile
{
    D3D12_BLEND_DESC blendState;
    D3D12_RASTERIZER_DESC rasterizerState;
    D3D12_DEPTH_STENCIL_DESC depthStencilState;

    D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType;
    std::vector<DXGI_FORMAT> rtvFormats;
    DXGI_FORMAT dsvFormat;
    DXGI_SAMPLE_DESC sampleDesc;

    PipelineProfile();

    bool operator<(const PipelineProfile& r) const;
};