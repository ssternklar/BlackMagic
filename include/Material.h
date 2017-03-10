#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "DirectionalLight.h"
#include "SimpleShader.h"
#include "Texture.h"

using BlackMagic::Texture;

enum ResourceStage : int
{
	VertexShader = 1 << 0,	
	HullShader = 1 << 1,
	DomainShader = 1 << 2,
	GeometryShader = 1 << 3,
	PixelShader = 1 << 4,
	ComputeShader = 1 << 5
};

enum ResourceType : int
{
	Data,
	Texture,
	Sampler
};

class Material
{
public:
	Material(
		const std::shared_ptr<SimpleVertexShader>& vs, 
		const std::shared_ptr<SimplePixelShader>& ps,
		const std::shared_ptr<SimpleHullShader>* hs = nullptr,
        const std::shared_ptr<SimpleDomainShader>* ds = nullptr,
        const std::shared_ptr<SimpleGeometryShader>* gs = nullptr
	);

	SimpleVertexShader* VertexShader() const;
	SimplePixelShader* PixelShader() const;
    SimpleHullShader* HullShader() const;
    SimpleDomainShader* DomainShader() const;
    SimpleGeometryShader* GeometryShader() const;

    //Turns on shader stages and uploads persistent data
    void Use() const;

	void SetData(std::string name, ResourceStage s, ResourceType t, size_t size, void* data, bool persistent = false);

private:
	struct ResourceData
	{
		ResourceStage stage;
		ResourceType type;
		size_t size;
		unsigned char* data;
	};

    bool _active = false;
	std::unordered_map<std::string, ResourceData> _persistentData;
	std::shared_ptr<SimpleVertexShader> _vertShader;
	std::shared_ptr<SimplePixelShader> _pixelShader;
    std::shared_ptr<SimpleHullShader> _hullShader;
    std::shared_ptr<SimpleDomainShader> _domainShader;
    std::shared_ptr<SimpleGeometryShader> _geometryShader;
};
