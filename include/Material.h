#pragma once

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

		enum class ResourceStorageType : unsigned char
		{
			Static,
			Instance,
			Frame
		};

		Material() = default;
		Material(
			BestFitAllocator& allocator,
			const std::shared_ptr<SimpleVertexShader>& vs, 
			const std::shared_ptr<SimplePixelShader>& ps
		);
		Material(const Material&);
		Material& operator=(const Material&);
		~Material();

		SimpleVertexShader* VertexShader() const;
		SimplePixelShader* PixelShader() const;

		//Turns on shader stages and uploads persistent data
		void Use(bool freshUse = false) const;
		void SetResource(std::string name, ResourceStage s, size_t size, void* data, ResourceStorageType storage = ResourceStorageType::Frame) const;
		void SetResource(std::string name, ResourceStage s, const std::shared_ptr<Texture>& tex, ResourceStorageType storage = ResourceStorageType::Frame) const;
		void SetResource(std::string name, ResourceStage s, const Sampler& sampler, ResourceStorageType storage = ResourceStorageType::Frame) const;
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


		using AllocatorAdapter = AllocatorSTLAdapter<std::pair<std::string, std::shared_ptr<ResourceData>>, BestFitAllocator>;
		mutable std::unordered_map<std::string, std::shared_ptr<ResourceData>, std::hash<std::string>, std::equal_to<std::string>, 
			AllocatorSTLAdapter<std::pair<std::string, std::shared_ptr<ResourceData>>, BestFitAllocator>> _staticData;
		mutable std::unordered_map<std::string, std::shared_ptr<ResourceData>, std::hash<std::string>, std::equal_to<std::string>,
			AllocatorSTLAdapter<std::pair<std::string, std::shared_ptr<ResourceData>>, BestFitAllocator>> _instanceData;
		std::shared_ptr<SimpleVertexShader> _vertShader;
		std::shared_ptr<SimplePixelShader> _pixelShader;
		BestFitAllocator* _allocator;

		void UploadData(std::string name, const ResourceData& dat) const;
	};
}
