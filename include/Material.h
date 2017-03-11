#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

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
			const std::shared_ptr<SimpleVertexShader>& vs, 
			const std::shared_ptr<SimplePixelShader>& ps,
			const std::shared_ptr<SimpleHullShader>* hs = nullptr,
			const std::shared_ptr<SimpleDomainShader>* ds = nullptr,
			const std::shared_ptr<SimpleGeometryShader>* gs = nullptr
		);
		~Material();

		SimpleVertexShader* VertexShader() const;
		SimplePixelShader* PixelShader() const;
		SimpleHullShader* HullShader() const;
		SimpleDomainShader* DomainShader() const;
		SimpleGeometryShader* GeometryShader() const;

		//Turns on shader stages and uploads persistent data
		void Use(bool dataOnly = false) const;
		void SetResource(std::string name, ResourceStage s, ResourceType t, size_t size, void* data, bool persistent = false) const;
		bool operator==(const Material& mat) const;
		bool operator!=(const Material& mat) const;

	private:
		struct ResourceData
		{
			ResourceStage stage;
			ResourceType type;
			size_t size;
			void* data;
		};

		mutable std::unordered_map<std::string, ResourceData> _persistentData;
		std::shared_ptr<SimpleVertexShader> _vertShader;
		std::shared_ptr<SimplePixelShader> _pixelShader;
		std::shared_ptr<SimpleHullShader> _hullShader;
		std::shared_ptr<SimpleDomainShader> _domainShader;
		std::shared_ptr<SimpleGeometryShader> _geometryShader;

		void UploadData(std::string name, const ResourceData& dat) const;
	};
}
