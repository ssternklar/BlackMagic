#include "Shaders.h"

const char* ShaderTypeString<ISimpleShader>::value = nullptr;
const char* ShaderTypeString<SimpleComputeShader>::value = "cs_5_0";
const char* ShaderTypeString<SimpleDomainShader>::value = "ds_5_0";
const char* ShaderTypeString<SimpleGeometryShader>::value = "gs_5_0";
const char* ShaderTypeString<SimpleHullShader>::value = "hs_5_0";
const char* ShaderTypeString<SimplePixelShader>::value = "ps_5_0";
const char* ShaderTypeString<SimpleVertexShader>::value = "vs_5_0";

ShaderData::ShaderData(ID3D11Device* device, ID3D11DeviceContext* context)
{
	this->device = device;
	this->context = context;
}

ShaderData::~ShaderData()
{

}