#include "Material.h"

Material::Material(
	const std::shared_ptr<SimpleVertexShader>& vs,
	const std::shared_ptr<SimplePixelShader>& ps,
	const std::shared_ptr<SimpleHullShader>* hs = nullptr,
	const std::shared_ptr<SimpleDomainShader>* ds = nullptr,
	const std::shared_ptr<SimpleGeometryShader>* gs = nullptr
)
	: _vertShader(vs),
	_pixelShader(ps),
	_hullShader(hs),
	_domainShader(ds),
	_geometryShader(gs)
{}


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

void Material::SetData(std::string name, ResourceStage s, ResourceType t, size_t size, void* data, bool persistent = false)
{
	if (persistent)
	{
		ResourceData dat;
		dat.stage = s;
		dat.size = size;
		dat.data = new unsigned char(size);
		memcpy(dat.data, data, size);
		_persistentData[name] = dat;	
	}
	else 
	{
		switch (t)
		{
		case ResourceType::Data:
			if (s & ResourceStage::VertexShader)
				_vertShader->SetData(name, data, size);
			if (s & ResourceStage::PixelShader)
				_pixelShader->SetData(name, data, size);
			if (_hullShader && s & ResourceStage::HullShader)
				_hullShader->SetData(name, data, size);
			if (_domainShader && s & ResourceStage::DomainShader)
				_domainShader->SetData(name, data, size);
			if (_geometryShader && s & ResourceStage::GeometryShader)
				_geometryShader->SetData(name, data, size);
			break;
		case ResourceType::Sampler:
			if (s & ResourceStage::VertexShader)
				_vertShader->SetData(name, data, size);
			if (s & ResourceStage::PixelShader)
				_pixelShader->SetData(name, data, size);
			if (_hullShader && s & ResourceStage::HullShader)
				_hullShader->SetData(name, data, size);
			if (_domainShader && s & ResourceStage::DomainShader)
				_domainShader->SetData(name, data, size);
			if (_geometryShader && s & ResourceStage::GeometryShader)
				_geometryShader->SetData(name, data, size);
			break;
		case ResourceType::Texture:
			if (s & ResourceStage::VertexShader)
				_vertShader->SetData(name, data, size);
			if (s & ResourceStage::PixelShader)
				_pixelShader->SetData(name, data, size);
			if (_hullShader && s & ResourceStage::HullShader)
				_hullShader->SetData(name, data, size);
			if (_domainShader && s & ResourceStage::DomainShader)
				_domainShader->SetData(name, data, size);
			if (_geometryShader && s & ResourceStage::GeometryShader)
				_geometryShader->SetData(name, data, size);
			break;
		}
		
	}
}

void Material::Use() const
{
	_vertShader->SetShader();
}

bool Material::operator==(const Material& mat) const
{
	return false;
}
