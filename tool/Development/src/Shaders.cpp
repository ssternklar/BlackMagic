#include "Shaders.h"
#include "FileUtil.h"
#include "Assets.h"

const char* ShaderTypeString<Shader::Compute>::value = "cs_5_0";
const char* ShaderTypeString<Shader::Domain>::value = "ds_5_0";
const char* ShaderTypeString<Shader::Geometry>::value = "gs_5_0";
const char* ShaderTypeString<Shader::Hull>::value = "hs_5_0";
const char* ShaderTypeString<Shader::Pixel>::value = "ps_5_0";
const char* ShaderTypeString<Shader::Vertex>::value = "vs_5_0";

ShaderData::~ShaderData()
{
	for (size_t i = 0; i < size; ++i)
		delete data[i].shader;
}

void ShaderData::Init(ID3D11Device* device, ID3D11DeviceContext* context)
{
	this->device = device;
	this->context = context;
}

ShaderData::Handle ShaderData::Get(std::string shaderPath, Shader::Type type)
{
	std::string fullPath = root + shaderPath;

	Handle h = AssetManager::Instance().GetHandle<ShaderData>(fullPath);
	if (h.ptr())
		return h;

	// this sucks
	// can't template this function because it needs "Assets.h" which causes cyclic includes in the header
	switch (type)
	{
	case Shader::Compute:
		h = Load<Shader::Compute>(fullPath);
		break;
	case Shader::Domain:
		h = Load<Shader::Domain>(fullPath);
		break;
	case Shader::Geometry:
		h = Load<Shader::Geometry>(fullPath);
		break;
	case Shader::Hull:
		h = Load<Shader::Hull>(fullPath);
		break;
	case Shader::Pixel:
		h = Load<Shader::Pixel>(fullPath);
		break;
	case Shader::Vertex:
		h = Load<Shader::Vertex>(fullPath);
		break;
	}

	if (!h.ptr())
		return h;

	AssetManager::Instance().TrackAsset<ShaderData>(h, fullPath);

	return h;
}

void ShaderData::Revoke(Handle handle)
{
	delete handle->shader;

	AssetManager::Instance().StopTrackingAsset<ShaderData>(handle);
	ProxyHandler::Revoke(handle);
}

void ShaderData::Export(std::string path, Handle handle)
{
	FileUtil::CopyFileViaPaths(AssetManager::Instance().GetAsset<ShaderData>(handle).path, path);
}
