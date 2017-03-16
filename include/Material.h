#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "allocators/AllocatorSTLAdapter.h"
#include "allocators/BadBestFitAllocator.h"
#include "DirectionalLight.h"
#include "SimpleShader.h"
#include "Texture.h"


namespace BlackMagic
{
	class Material
	{
	public:
		enum ResourceStage : unsigned char
		{
			VS = 1 << 0,
			HS = 1 << 1,
			DS = 1 << 2,
			GS = 1 << 3,
			PS = 1 << 4,
			CS = 1 << 5
		};

		enum class ResourceType : unsigned char
		{
			Data,
			Texture,
			Sampler
		};

		Material() = default;
		Material(
			BestFitAllocator& allocator,
			const std::shared_ptr<SimpleVertexShader>& vs, 
			const std::shared_ptr<SimplePixelShader>& ps,
			const std::shared_ptr<SimpleHullShader>* hs = nullptr,
			const std::shared_ptr<SimpleDomainShader>* ds = nullptr,
			const std::shared_ptr<SimpleGeometryShader>* gs = nullptr
		);
		Material(const Material&);
		Material& operator=(const Material&);
		~Material();

		SimpleVertexShader* VertexShader() const;
		SimplePixelShader* PixelShader() const;
		SimpleHullShader* HullShader() const;
		SimpleDomainShader* DomainShader() const;
		SimpleGeometryShader* GeometryShader() const;

		//Turns on shader stages and uploads persistent data
		void Use(bool dataOnly = false) const;
		void SetResource(std::string name, ResourceStage s, size_t size, void* data, bool persistent = false) const;
		void SetResource(std::string name, ResourceStage s, const std::shared_ptr<Texture>& tex, bool persistent = false) const;
		void SetResource(std::string name, ResourceStage s, const Sampler& sampler, bool persistent = false) const;
		bool operator==(const Material& mat) const;
		bool operator!=(const Material& mat) const;

	private:
		struct ResourceData
		{
			ResourceStage stage;
			ResourceType type;
			size_t size;
			void* data;

			~ResourceData()
			{
				switch (type)
				{
					case ResourceType::Texture:
						static_cast<std::shared_ptr<Texture>*>(data)->reset();
						break;
					case ResourceType::Sampler:
						static_cast<Sampler*>(data)->~Sampler();
						break;
					case ResourceType::Data:
						break;
				}
			}
		};

		mutable std::unordered_map<std::string, std::shared_ptr<ResourceData>, std::hash<std::string>, std::equal_to<std::string>, 
			AllocatorSTLAdapter<std::pair<std::string, std::shared_ptr<ResourceData>>, BestFitAllocator>> _persistentData;
		std::shared_ptr<SimpleVertexShader> _vertShader;
		std::shared_ptr<SimplePixelShader> _pixelShader;
		std::shared_ptr<SimpleHullShader> _hullShader;
		std::shared_ptr<SimpleDomainShader> _domainShader;
		std::shared_ptr<SimpleGeometryShader> _geometryShader;
		BestFitAllocator* _allocator;

		size_t GetTotalResourceMem(ISimpleShader* shader);
		void UploadData(std::string name, const ResourceData& dat) const;
	};
}
