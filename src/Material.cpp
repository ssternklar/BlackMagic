#include "Material.h"

#if defined(_WIN32) || defined(_WIN64)
#include "WindowsPlatform.h"
#endif

using namespace BlackMagic;

Material::Material(
	const std::shared_ptr<SimpleVertexShader>& vs,
	const std::shared_ptr<SimplePixelShader>& ps,
	const std::shared_ptr<SimpleHullShader>* hs,
	const std::shared_ptr<SimpleDomainShader>* ds,
	const std::shared_ptr<SimpleGeometryShader>* gs
)
	: _vertShader(vs),
	_pixelShader(ps),
	_hullShader(hs ? *hs : nullptr),
	_domainShader(ds ? *ds : nullptr),
	_geometryShader(gs ? *gs : nullptr)
{}

Material::~Material()
{
	for (auto& p : _persistentData)
	{
		switch (p.second.type)
		{
		case ResourceType::Data:
			delete p.second.data;
			break;
		case ResourceType::Texture:
		case ResourceType::Sampler:
#if defined(_WIN32) || defined(_WIN64)
			WindowsPlatform::GetInstance()->GetRenderer()->ReleaseResource(p.second.data);
#endif

		}
	}
}


SimpleVertexShader* Material::VertexShader() const
{
	return _vertShader.get();
}

SimplePixelShader* Material::PixelShader() const
{
	return _pixelShader.get();
}

SimpleHullShader* Material::HullShader() const
{
	return _hullShader.get();
}

SimpleDomainShader* Material::DomainShader() const
{
	return _domainShader.get();
}

SimpleGeometryShader* Material::GeometryShader() const
{
	return _geometryShader.get();
}

void Material::SetResource(std::string name, ResourceStage s, ResourceType t, size_t size, void* data, bool persistent) const
{
	ResourceData dat;
	dat.stage = s;
	dat.size = size;
	
	//If the data is persistent for this material instance, just insert it into the map
	//Persistent data gets uploaded in Use
	//Otherwise, this is per-frame data so upload it to the shaders immediately
	if (persistent)
	{	
		dat.data = new unsigned char(size);
		memcpy(dat.data, data, size);
		_persistentData[name] = dat;	
	}
	else
	{
		dat.data = data;
		UploadData(name, dat);
	}
}

void Material::Use(bool dataOnly) const
{
	if (!dataOnly)
	{
		_vertShader->SetShader();
		_pixelShader->SetShader();

		if (_hullShader && _domainShader)
		{
			_hullShader->SetShader();
			_domainShader->SetShader();
		}

		if (_geometryShader)
			_geometryShader->SetShader();
	
	}
	//Need to reupload persistent data on each Use because SimpleShader instances are shared
	for(auto& pair : _persistentData)
	{
		UploadData(pair.first, pair.second);
	}

	if (!dataOnly)
	{
		//TODO: Seperate per-frame and per-instance uploads
		_vertShader->CopyAllBufferData();
		_pixelShader->CopyAllBufferData();

		if (_hullShader && _domainShader)
		{
			_hullShader->CopyAllBufferData();
			_domainShader->CopyAllBufferData();
		}

		if (_geometryShader)
			_geometryShader->CopyAllBufferData();	
	}

}

bool Material::operator==(const Material& mat) const
{
	return (_vertShader.get() == mat._vertShader.get()
		&& _pixelShader.get() == mat._pixelShader.get()
		&& _hullShader.get() == mat._hullShader.get()
		&& _domainShader.get() == mat._domainShader.get()
		&& _geometryShader.get() == mat._geometryShader.get());
}

bool Material::operator!=(const Material& mat) const
{
	return !(*this == mat);
}

void Material::UploadData(std::string name, const ResourceData& dat) const
{
	auto s = dat.stage;
	switch (dat.type)
	{
	case ResourceType::Data:
		if (s & ResourceStage::VS)
			_vertShader->SetData(name, dat.data, dat.size);
		if (s & ResourceStage::PS)
			_pixelShader->SetData(name, dat.data, dat.size);
		if (_hullShader && s & ResourceStage::HS)
			_hullShader->SetData(name, dat.data, dat.size);
		if (_domainShader && s & ResourceStage::DS)
			_domainShader->SetData(name, dat.data, dat.size);
		if (_geometryShader && s & ResourceStage::GS)
			_geometryShader->SetData(name, dat.data, dat.size);
		break;
	case ResourceType::Sampler:
		if (s & ResourceStage::VS)
			_vertShader->SetSamplerState(name, reinterpret_cast<ID3D11SamplerState*>(dat.data));
		if (s & ResourceStage::PS)
			_pixelShader->SetSamplerState(name, reinterpret_cast<ID3D11SamplerState*>(dat.data));
		if (_hullShader && s & ResourceStage::HS)
			_hullShader->SetSamplerState(name, reinterpret_cast<ID3D11SamplerState*>(dat.data));
		if (_domainShader && s & ResourceStage::DS)
			_domainShader->SetSamplerState(name, reinterpret_cast<ID3D11SamplerState*>(dat.data));
		if (_geometryShader && s & ResourceStage::GS)
			_geometryShader->SetSamplerState(name, reinterpret_cast<ID3D11SamplerState*>(dat.data));
		break;
	case ResourceType::Texture:
		if (s & ResourceStage::VS)
			_vertShader->SetShaderResourceView(name, reinterpret_cast<ID3D11ShaderResourceView*>(dat.data));
		if (s & ResourceStage::PS)
			_pixelShader->SetShaderResourceView(name, reinterpret_cast<ID3D11ShaderResourceView*>(dat.data));
		if (_hullShader && s & ResourceStage::HS)
			_hullShader->SetShaderResourceView(name, reinterpret_cast<ID3D11ShaderResourceView*>(dat.data));
		if (_domainShader && s & ResourceStage::DS)
			_domainShader->SetShaderResourceView(name, reinterpret_cast<ID3D11ShaderResourceView*>(dat.data));
		if (_geometryShader && s & ResourceStage::GS)
			_geometryShader->SetShaderResourceView(name, reinterpret_cast<ID3D11ShaderResourceView*>(dat.data));
		break;
	}
}