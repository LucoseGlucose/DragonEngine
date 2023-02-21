#include "stdafx.h"
#include "ShaderProgram.h"

#include "Rendering.h"
#include <d3d12shader.h>
#include <d3dcompiler.h>

ShaderProgram::ShaderProgram(const std::map<SHADER_TYPE, Shader*>& shaderList, uint32_t samples, DXGI_FORMAT format) : shaders(shaderList)
{
    uint32_t numConstantBuffers = 0;
    uint32_t numTextures = 0;
    uint32_t numSamplers = 0;

    std::vector<D3D12_ROOT_PARAMETER> rootParams{};
    std::vector<D3D12_ROOT_PARAMETER> textureParams{};
    std::vector<D3D12_ROOT_PARAMETER> samplerParams{};

    std::vector<D3D12_DESCRIPTOR_RANGE*> rangePtrs{};

    D3D12_SHADER_DESC vertexShaderDesc;
    ComPtr<ID3D12ShaderReflection> vertexShaderReflection;

    for (size_t i = 0; i < 2; i++)
    {
        SHADER_TYPE type = (SHADER_TYPE)i;
        if (!shaders.contains(type)) continue;
        Shader* shader = shaders[type];

        ComPtr<ID3D12ShaderReflection> shaderReflection;
        Utils::ThrowIfFailed(D3DReflect(shader->bytecode.pShaderBytecode,
            shader->bytecode.BytecodeLength, IID_PPV_ARGS(&shaderReflection)));
        if (type == SHADER_TYPE_VERTEX) vertexShaderReflection = shaderReflection;

        D3D12_SHADER_DESC shaderDesc{};
        Utils::ThrowIfFailed(shaderReflection->GetDesc(&shaderDesc));
        if (type == SHADER_TYPE_VERTEX) vertexShaderDesc = shaderDesc;

        for (size_t res = 0; res < shaderDesc.BoundResources; res++)
        {
            D3D12_SHADER_INPUT_BIND_DESC bindDesc;
            Utils::ThrowIfFailed(shaderReflection->GetResourceBindingDesc(res, &bindDesc));

            if (bindDesc.Type == D3D_SIT_CBUFFER)
            {
                D3D12_ROOT_DESCRIPTOR rootDesc{};
                rootDesc.ShaderRegister = numConstantBuffers;
                rootDesc.RegisterSpace = 0;

                D3D12_ROOT_PARAMETER rootParam{};
                rootParam.Descriptor = rootDesc;
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

                rootParams.push_back(rootParam);
                numConstantBuffers++;
            }
            if (bindDesc.Type == D3D_SIT_TEXTURE)
            {
                D3D12_DESCRIPTOR_RANGE* descRange = new D3D12_DESCRIPTOR_RANGE();
                descRange->BaseShaderRegister = numTextures;
                descRange->NumDescriptors = 1;
                descRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                descRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                descRange->RegisterSpace = 0;

                rangePtrs.push_back(descRange);

                D3D12_ROOT_DESCRIPTOR_TABLE descTable{};
                descTable.NumDescriptorRanges = 1;
                descTable.pDescriptorRanges = descRange;

                D3D12_ROOT_PARAMETER rootParam{};
                rootParam.DescriptorTable = descTable;
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

                textureParams.push_back(rootParam);
                numTextures++;
            }
            if (bindDesc.Type == D3D_SIT_SAMPLER)
            {
                D3D12_DESCRIPTOR_RANGE* descRange = new D3D12_DESCRIPTOR_RANGE();
                descRange->BaseShaderRegister = numSamplers;
                descRange->NumDescriptors = 1;
                descRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                descRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                descRange->RegisterSpace = 0;

                rangePtrs.push_back(descRange);

                D3D12_ROOT_DESCRIPTOR_TABLE descTable{};
                descTable.NumDescriptorRanges = 1;
                descTable.pDescriptorRanges = descRange;

                D3D12_ROOT_PARAMETER rootParam{};
                rootParam.DescriptorTable = descTable;
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

                samplerParams.push_back(rootParam);
                numSamplers++;
            }
        }
    }

    uint32_t numParams = rootParams.size() + textureParams.size() + samplerParams.size();
    D3D12_ROOT_PARAMETER* allParams = new D3D12_ROOT_PARAMETER[numParams];
    D3D12_ROOT_PARAMETER* ptrCpy = allParams;

    memcpy(allParams, rootParams.data(), rootParams.size() * sizeof(D3D12_ROOT_PARAMETER));
    allParams += rootParams.size();
    memcpy(allParams, textureParams.data(), textureParams.size() * sizeof(D3D12_ROOT_PARAMETER));
    allParams += textureParams.size();
    memcpy(allParams, samplerParams.data(), samplerParams.size() * sizeof(D3D12_ROOT_PARAMETER));

    CD3DX12_ROOT_SIGNATURE_DESC rsDesc{};
    rsDesc.Init(numParams, ptrCpy, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS);

    ComPtr<ID3DBlob> rsBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT result = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsBlob, &errorBlob);
    if (FAILED(result) || errorBlob != nullptr)
    {
        std::string error = std::string((char*)errorBlob->GetBufferPointer());
        throw std::runtime_error(error);
    }

    Utils::ThrowIfFailed(Rendering::device->CreateRootSignature(0,
        rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
    NAME_D3D_OBJECT(rootSignature);

    for (size_t i = 0; i < rangePtrs.size(); i++)
    {
        delete rangePtrs[i];
    }
    delete[] ptrCpy;

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;

    for (size_t i = 0; i < vertexShaderDesc.InputParameters; i++)
    {
        D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
        vertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

        D3D12_INPUT_ELEMENT_DESC elementDesc{};
        elementDesc.SemanticName = paramDesc.SemanticName;
        elementDesc.SemanticIndex = paramDesc.SemanticIndex;
        elementDesc.InputSlot = 0;
        elementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        elementDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        elementDesc.InstanceDataStepRate = 0;

        if (paramDesc.Mask == 1)
        {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
        }
        else if (paramDesc.Mask <= 3)
        {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
        }
        else if (paramDesc.Mask <= 7)
        {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        }
        else if (paramDesc.Mask <= 15)
        {
            if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
            else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }

        inputLayout.push_back(elementDesc);
    }

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.NumElements = inputLayout.size();
    inputLayoutDesc.pInputElementDescs = inputLayout.data();

    DXGI_SAMPLE_DESC sampleDesc{};
    sampleDesc.Count = samples;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
    pipelineDesc.InputLayout = inputLayoutDesc;
    pipelineDesc.pRootSignature = rootSignature.Get();
    pipelineDesc.VS = shaders[SHADER_TYPE_VERTEX]->bytecode;
    pipelineDesc.PS = shaders[SHADER_TYPE_PIXEL]->bytecode;
    pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineDesc.RTVFormats[0] = format;
    pipelineDesc.SampleDesc = sampleDesc;
    pipelineDesc.SampleMask = 0xffffffff;
    pipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pipelineDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pipelineDesc.NumRenderTargets = 1;
    pipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    Utils::ThrowIfFailed(Rendering::device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipeline)));
    NAME_D3D_OBJECT(pipeline);
}

ShaderProgram* ShaderProgram::Create(const std::map<SHADER_TYPE, Shader*>& shaderList, uint32_t samples, DXGI_FORMAT format)
{
    if (createdPrograms.contains(shaderList)) return createdPrograms[shaderList];
    return new ShaderProgram(shaderList, samples, format);
}

ShaderProgram* ShaderProgram::Create(const std::filesystem::path& vertexShader,
    const std::filesystem::path& pixelShader, uint32_t samples, DXGI_FORMAT format)
{
    std::map<SHADER_TYPE, Shader*> shaderList = std::map<SHADER_TYPE, Shader*>();

    shaderList[SHADER_TYPE_VERTEX] = Shader::Create(vertexShader, SHADER_TYPE_VERTEX);
    shaderList[SHADER_TYPE_PIXEL] = Shader::Create(pixelShader, SHADER_TYPE_PIXEL);

    if (createdPrograms.contains(shaderList)) return createdPrograms[shaderList];
    return new ShaderProgram(shaderList, samples, format);
}
