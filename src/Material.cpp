#include "Material.h"

#if defined(BM_PLATFORM_WINDOWS)
#include "WindowsPlatform.h"
#endif

using namespace BlackMagic;

Material::Material(
	BestFitAllocator& allocator,
	const std::shared_ptr<SimpleVertexShader>& vs,
	const std::shared_ptr<SimplePixelShader>& ps
)
	: _vertShader(vs),
	_pixelShader(ps),
	_allocator(&allocator),
	_staticData(&allocator),
	_instanceData(&allocator)
{

}

Material::Material(const Material& m)
	: Material(*m._allocator, m._vertShader, m._pixelShader)
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
		dat.type = ResourceType::Sampler;
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
}

bool Material::operator==(const Material& mat) const
{
	//Not including _instanceData because we only care if these materials have the same
	//base parameters (_globalData)
	return (_vertShader.get() == mat._vertShader.get()
		&& _pixelShader.get() == mat._pixelShader.get()
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
		break;
	case ResourceType::Sampler:
		if (s & ResourceStage::VS)
			_vertShader->SetSamplerState(name, *static_cast<Sampler*>(dat.data));
		if (s & ResourceStage::PS)
			_pixelShader->SetSamplerState(name, *static_cast<Sampler*>(dat.data));
		break;
	case ResourceType::Texture:
		if (s & ResourceStage::VS)
			_vertShader->SetShaderResourceView(name, **static_cast<std::shared_ptr<Texture>*>(dat.data));
		if (s & ResourceStage::PS)
			_pixelShader->SetShaderResourceView(name, **static_cast<std::shared_ptr<Texture>*>(dat.data));
		break;
	}
}