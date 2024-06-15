#include "stdafx.h"
#include "ShaderProgram.h"

#include "Rendering.h"
#include <d3d12shader.h>
#include <dxcapi.h>

ShaderProgram::ShaderProgram(const std::map<SHADER_TYPE, Shader*>& shaderList) : shaders(shaderList)
{
    UINT32 numConstantBuffers = 0;
    UINT32 numTextures = 0;
    UINT32 numSamplers = 0;
    UINT32 numUAVs = 0;

    std::vector<D3D12_ROOT_PARAMETER> rootParams{};
    std::vector<D3D12_ROOT_PARAMETER> textureParams{};
    std::vector<D3D12_ROOT_PARAMETER> samplerParams{};
    std::vector<D3D12_ROOT_PARAMETER> uavParams{};

    std::vector<D3D12_DESCRIPTOR_RANGE*> rangePtrs{};

    D3D12_SHADER_DESC vertexShaderDesc;
    ComPtr<ID3D12ShaderReflection> vertexShaderReflection;

    for (size_t i = 0; i < SHADER_TYPE_MAX; i++)
    {
        SHADER_TYPE type = (SHADER_TYPE)i;
        if (!shaders.contains(type)) continue;
        Shader* shader = shaders[type];

        ComPtr<IDxcUtils> utils;
        Utils::ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));

        DxcBuffer shaderBuffer{};
        shaderBuffer.Ptr = shader->bytecode.pShaderBytecode;
        shaderBuffer.Size = shader->bytecode.BytecodeLength;
        shaderBuffer.Encoding = 0u;

        ComPtr<ID3D12ShaderReflection> shaderReflection;
        Utils::ThrowIfFailed(utils->CreateReflection(&shaderBuffer, IID_PPV_ARGS(&shaderReflection)));

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
            if (bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED || bindDesc.Type == D3D_SIT_UAV_RWTYPED || bindDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS)
            {
                D3D12_DESCRIPTOR_RANGE* descRange = new D3D12_DESCRIPTOR_RANGE();
                descRange->BaseShaderRegister = numUAVs;
                descRange->NumDescriptors = 1;
                descRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                descRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                descRange->RegisterSpace = 0;

                rangePtrs.push_back(descRange);

                D3D12_ROOT_DESCRIPTOR_TABLE descTable{};
                descTable.NumDescriptorRanges = 1;
                descTable.pDescriptorRanges = descRange;

                D3D12_ROOT_PARAMETER rootParam{};
                rootParam.DescriptorTable = descTable;
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

                uavParams.push_back(rootParam);
                numUAVs++;
            }
        }
    }

    UINT32 numParams = rootParams.size() + textureParams.size() + samplerParams.size() + uavParams.size();
    D3D12_ROOT_PARAMETER* allParams = new D3D12_ROOT_PARAMETER[numParams];
    D3D12_ROOT_PARAMETER* ptrCopy = allParams;

    memcpy(allParams, rootParams.data(), rootParams.size() * sizeof(D3D12_ROOT_PARAMETER));
    allParams += rootParams.size();
    memcpy(allParams, textureParams.data(), textureParams.size() * sizeof(D3D12_ROOT_PARAMETER));
    allParams += textureParams.size();
    memcpy(allParams, samplerParams.data(), samplerParams.size() * sizeof(D3D12_ROOT_PARAMETER));
    allParams += samplerParams.size();
    memcpy(allParams, uavParams.data(), uavParams.size() * sizeof(D3D12_ROOT_PARAMETER));

    CD3DX12_ROOT_SIGNATURE_DESC rsDesc{};
    rsDesc.Init(numParams, ptrCopy, 0, nullptr,
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
    delete[] ptrCopy;

    D3D12_INPUT_ELEMENT_DESC* inputs = new D3D12_INPUT_ELEMENT_DESC[vertexShaderDesc.InputParameters];

    for (size_t i = 0; i < vertexShaderDesc.InputParameters; i++)
    {
        D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
        vertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

        D3D12_INPUT_ELEMENT_DESC elementDesc{};

        SIZE_T size = sizeof(paramDesc.SemanticName) + 1;
        elementDesc.SemanticName = new char[size];
        memcpy((void*)elementDesc.SemanticName, (void*)paramDesc.SemanticName, size);

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

        inputs[i] = elementDesc;
    }

    inputLayoutDesc.NumElements = vertexShaderDesc.InputParameters;
    inputLayoutDesc.pInputElementDescs = inputs;
}

ShaderProgram::~ShaderProgram()
{
    for (size_t i = 0; i < inputLayoutDesc.NumElements; i++)
    {
        delete[] inputLayoutDesc.pInputElementDescs[i].SemanticName;
    }

    delete[] inputLayoutDesc.pInputElementDescs;
}

void ShaderProgram::CompileShaderForProfile(const PipelineProfile& profile)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
    pipelineDesc.InputLayout = inputLayoutDesc;
    pipelineDesc.pRootSignature = rootSignature.Get();
    pipelineDesc.VS = shaders[SHADER_TYPE_VERTEX]->bytecode;
    pipelineDesc.PS = shaders[SHADER_TYPE_PIXEL]->bytecode;
    pipelineDesc.SampleMask = 0xffffffff;

    pipelineDesc.PrimitiveTopologyType = profile.topologyType;
    pipelineDesc.NumRenderTargets = min(8, profile.rtvFormats.size());

    for (size_t i = 0; i < pipelineDesc.NumRenderTargets; i++)
    {
        pipelineDesc.RTVFormats[i] = profile.rtvFormats[i];
    }

    pipelineDesc.SampleDesc = profile.sampleDesc;
    pipelineDesc.RasterizerState = profile.rasterizerState;
    pipelineDesc.BlendState = profile.blendState;
    pipelineDesc.DepthStencilState = profile.depthStencilState;
    pipelineDesc.DSVFormat = profile.dsvFormat;

    ComPtr<ID3D12PipelineState> pipeline;
    Utils::ThrowIfFailed(Rendering::device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipeline)));
    NAME_D3D_OBJECT(pipeline);

    compiledPipelines.emplace(profile, pipeline);
}

ShaderProgram* ShaderProgram::Create(const std::map<SHADER_TYPE, Shader*>& shaderList)
{
    if (createdPrograms.contains(shaderList)) return createdPrograms[shaderList];
    return new ShaderProgram(shaderList);
}

ShaderProgram* ShaderProgram::Create(const std::filesystem::path& vertexShader, const std::filesystem::path& pixelShader)
{
    std::map<SHADER_TYPE, Shader*> shaderList = std::map<SHADER_TYPE, Shader*>();

    shaderList[SHADER_TYPE_VERTEX] = Shader::Create(vertexShader, SHADER_TYPE_VERTEX);
    shaderList[SHADER_TYPE_PIXEL] = Shader::Create(pixelShader, SHADER_TYPE_PIXEL);

    return Create(shaderList);
}
