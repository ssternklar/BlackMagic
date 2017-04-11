#include "Material.h"

#if defined(BM_PLATFORM_WINDOWS)
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
	_staticData(&allocator),
	_instanceData(&allocator)
{

}

Material::Material(const Material& m)
	: Material(*m._allocator, m._vertShader, m._pixelShader, &m._hullShader, &m._domainShader, &m._geometryShader)
{
	_staticData = m._staticData;
	_instanceData = m._instanceData;
}

Material::~Material()
{
	
}

Material& Material::operator=(const Material& m)
{
	_allocator = m._allocator;
	_vertShader = m._vertShader;
	_pixelShader = m._pixelShader;
	_hullShader = m._hullShader;
	_domainShader = m._domainShader;
	_geometryShader = m._geometryShader;
	_staticData = m._staticData;
	_instanceData = m._instanceData;

	return *this;
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

void Material::SetResource(std::string name, ResourceStage s, size_t size, void* data, ResourceStorageType storage) const
{
	//If the data isn't persistent (constant across all material instances) send it straight to the shaders
	//Else, save it into the persistent data pool
	if (storage == ResourceStorageType::Frame)
	{
		ResourceData dat;
		dat.stage = s;
		dat.size = size;
		dat.type = ResourceType::Data;
		dat.data = data;
		UploadData(name, dat);
	}
	else
	{
		auto dat = std::allocate_shared<ResourceData>(AllocatorSTLAdapter<ResourceData, BestFitAllocator>(_allocator));
		dat->stage = s;
		dat->size = size;
		dat->type = ResourceType::Data;
		dat->data = _allocator->allocate(dat->size);
		memcpy(dat->data, data, size);
		if (storage == ResourceStorageType::Static)
			_staticData[name] = dat;
		else
			_instanceData[name] = dat;
	}
}

void Material::SetResource(std::string name, ResourceStage s, const std::shared_ptr<Texture>& tex, ResourceStorageType storage) const
{

	if (storage == ResourceStorageType::Frame)
	{
		ResourceData dat;
		dat.stage = s;
		dat.type = ResourceType::Texture;
		dat.data = tex.get();
		dat.size = sizeof(std::shared_ptr<Texture>);
		UploadData(name, dat);
	}
	else
	{
		auto dat = std::allocate_shared<ResourceData>(AllocatorSTLAdapter<ResourceData, BestFitAllocator>(_allocator));
		dat->stage = s;
		dat->size = sizeof(std::shared_ptr<Texture>);
		dat->type = ResourceType::Texture;
		dat->data = _allocator->allocate(dat->size);
		new (dat->data) std::shared_ptr<Texture>(tex);
		if (storage == ResourceStorageType::Static)
			_staticData[name] = dat;
		else
			_instanceData[name] = dat;
	}
}

void Material::SetResource(std::string name, ResourceStage s, const Sampler& sampler, ResourceStorageType storage) const
{
	if (storage == ResourceStorageType::Frame)
	{
		ResourceData dat;
		dat.stage = s;
		dat.type = ResourceType::Texture;
		dat.data = sampler.As<SamplerHandle>();
		dat.size = sizeof(SamplerHandle);
		UploadData(name, dat);
	}
	else
	{
		auto dat = std::allocate_shared<ResourceData>(AllocatorSTLAdapter<ResourceData, BestFitAllocator>(_allocator));
		dat->stage = s;
		dat->size = sizeof(Sampler);
		dat->type = ResourceType::Sampler;
		dat->data = _allocator->allocate(dat->size);
		new (dat->data) Sampler(sampler);
		if (storage == ResourceStorageType::Static)
			_staticData[name] = dat;
		else
			_instanceData[name] = dat;
	}
}

void Material::Use(bool freshUse) const
{
	if (freshUse)
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
	
		//If freshUse == true, then all data needs to be set because it's a complete context switch
		//If false, a material sharing the same shader set and static data has already been set,
		//so none of that data needs to be uploaded
		for (auto& pair : _staticData)
		{
			UploadData(pair.first, *pair.second);
		}
	}
	
	for(auto& pair : _instanceData)
	{
		UploadData(pair.first, *pair.second);
	}

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

bool Material::operator==(const Material& mat) const
{
	//Not including _instanceData because we only care if these materials have the same
	//base parameters (_globalData)
	return (_vertShader.get() == mat._vertShader.get()
		&& _pixelShader.get() == mat._pixelShader.get()
		&& _hullShader.get() == mat._hullShader.get()
		&& _domainShader.get() == mat._domainShader.get()
		&& _geometryShader.get() == mat._geometryShader.get()
		&& _staticData == mat._staticData);
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
			_vertShader->SetData(name, dat.data, (uint32_t)dat.size);
		if (s & ResourceStage::PS)
			_pixelShader->SetData(name, dat.data, (uint32_t)dat.size);
		if (_hullShader && s & ResourceStage::HS)
			_hullShader->SetData(name, dat.data, (uint32_t)dat.size);
		if (_domainShader && s & ResourceStage::DS)
			_domainShader->SetData(name, dat.data, (uint32_t)dat.size);
		if (_geometryShader && s & ResourceStage::GS)
			_geometryShader->SetData(name, dat.data, (uint32_t)dat.size);
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

	for (unsigned int i = 0; i < shader->GetBufferCount(); i++)
	{
		buffers += shader->GetBufferSize(i);
	}

	return buffers + srvs + samplers;
}

