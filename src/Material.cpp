#include "Material.h"

#if defined(_WIN32) || defined(_WIN64)
#include "WindowsPlatform.h"
#endif

using namespace BlackMagic;

Material::Material(
	BestFitAllocator& allocator,
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
	_geometryShader(gs ? *gs : nullptr),
	_allocator(&allocator),
	_persistentData(&allocator)
{

}

Material::Material(const Material& m)
	: Material(*m._allocator, m._vertShader, m._pixelShader, &m._hullShader, &m._domainShader, &m._geometryShader)
{
	for (auto& p : m._persistentData)
	{
		switch (p.second.type)
		{
		case ResourceType::Data:
			SetResource(p.first, p.second.stage, p.second.size, p.second.data, true);
			break;
		case ResourceType::Texture:
			SetResource(p.first, p.second.stage, *static_cast<std::shared_ptr<Texture>*>(p.second.data), true);
			break;
		case ResourceType::Sampler:
			SetResource(p.first, p.second.stage, *static_cast<Sampler*>(p.second.data), true);
			break;
		}
	}
}

Material::~Material()
{
	for (auto& p : _persistentData)
	{
		switch (p.second.type)
		{
		case ResourceType::Texture:
			static_cast<std::shared_ptr<Texture>*>(p.second.data)->reset();
			break;
		case ResourceType::Sampler:
			static_cast<Sampler*>(p.second.data)->~Sampler();
			break;
		case ResourceType::Data:
			//Do nothing, don't need to run any destructor
			break;
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

void Material::SetResource(std::string name, ResourceStage s, size_t size, void* data, bool persistent) const
{
	ResourceData dat;
	dat.stage = s;
	dat.size = size;
	dat.type = ResourceType::Data;

	//If the data is persistent for this material instance, just insert it into the map
	//Persistent data gets uploaded in Use
	//Otherwise, this is per-frame data so upload it to the shaders immediately
	if (persistent)
	{	
		dat.data = _allocator->allocate(dat.size);
		memcpy(dat.data, data, size);
		_persistentData[name] = dat;	
	}
	else
	{
		dat.data = data;
		UploadData(name, dat);
	}
}

void Material::SetResource(std::string name, ResourceStage s, const std::shared_ptr<Texture>& tex, bool persistent) const
{
	ResourceData dat;
	dat.stage = s;
	dat.size = sizeof(std::shared_ptr<Texture>);
	dat.type = ResourceType::Texture;

	if (persistent)
	{
		dat.data = _allocator->allocate(dat.size);
		new (dat.data) std::shared_ptr<Texture>(tex);
		_persistentData[name] = dat;
	}
	else
	{
		dat.data = tex.get();
		UploadData(name, dat);
	}
}

void Material::SetResource(std::string name, ResourceStage s, const Sampler& sampler, bool persistent) const
{
	ResourceData dat;
	dat.stage = s;
	dat.size = sizeof(Sampler);
	dat.type = ResourceType::Sampler;

	if (persistent)
	{
		dat.data = _allocator->allocate(dat.size);
		new (dat.data) Sampler(sampler);
		_persistentData[name] = dat;
	}
	else
	{
		dat.data = sampler.As<SamplerHandle>();
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
			_vertShader->SetSamplerState(name, *static_cast<Sampler*>(dat.data));
		if (s & ResourceStage::PS)
			_pixelShader->SetSamplerState(name, *static_cast<Sampler*>(dat.data));
		if (_hullShader && s & ResourceStage::HS)
			_hullShader->SetSamplerState(name, *static_cast<Sampler*>(dat.data));
		if (_domainShader && s & ResourceStage::DS)
			_domainShader->SetSamplerState(name, *static_cast<Sampler*>(dat.data));
		if (_geometryShader && s & ResourceStage::GS)
			_geometryShader->SetSamplerState(name, *static_cast<Sampler*>(dat.data));
		break;
	case ResourceType::Texture:
		if (s & ResourceStage::VS)
			_vertShader->SetShaderResourceView(name, **static_cast<std::shared_ptr<Texture>*>(dat.data));
		if (s & ResourceStage::PS)
			_pixelShader->SetShaderResourceView(name, **static_cast<std::shared_ptr<Texture>*>(dat.data));
		if (_hullShader && s & ResourceStage::HS)
			_hullShader->SetShaderResourceView(name, **static_cast<std::shared_ptr<Texture>*>(dat.data));
		if (_domainShader && s & ResourceStage::DS)
			_domainShader->SetShaderResourceView(name, **static_cast<std::shared_ptr<Texture>*>(dat.data));
		if (_geometryShader && s & ResourceStage::GS)
			_geometryShader->SetShaderResourceView(name, **static_cast<std::shared_ptr<Texture>*>(dat.data));
		break;
	}
}

size_t Material::GetTotalResourceMem(ISimpleShader* shader)
{
	size_t buffers = 0;
	size_t srvs = shader->GetShaderResourceViewCount() * sizeof(std::shared_ptr<Texture>);
	size_t samplers = shader->GetSamplerCount() * sizeof(Sampler);

	for (int i = 0; i < shader->GetBufferCount(); i++)
	{
		buffers += shader->GetBufferSize(i);
	}

	return buffers + srvs + samplers;
}

