#pragma once
#include "ContentClasses.h"
#include "allocators\BadBestFitAllocator.h"
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"
namespace BlackMagic
{
	class SceneDesc
	{
	public:
		SceneDesc(BlackMagic::byte* file) : fileHandle(file) {};
		class SceneFileEntity
		{
			float transformData[12];
			uint16_t meshID;
			uint16_t materialID;
			uint16_t typeID;
		};
		BlackMagic::byte* fileHandle;
	};

	class Scene
	{
	public:
		Scene(BlackMagic::BestFitAllocator* allocator);

		template<class T>
		using AllocatorAdapter = BlackMagic::AllocatorSTLAdapter<T, BlackMagic::BestFitAllocator>;
		template<class T>
		using BMList = std::vector<T, AllocatorAdapter<T>>;

		const unsigned int SIZE_OF_SCENE_FILE_ENTITY = 54;
		const unsigned int SIZE_OF_TRANSFORM_FLOATS = 48;

		BlackMagic::BestFitAllocator* alloc;
		BlackMagic::AssetPointer<BlackMagic::UnknownContentType>* sceneAssets;
		int sceneAssetCount;

		void Init(BlackMagic::AssetPointer<SceneDesc> desc);
		virtual void Update(float deltaTime) {};
		virtual void Draw(float deltaTime) {};
		virtual void ProcessType(uint16_t tag, Transform transform, BlackMagic::AssetPointer<BlackMagic::Mesh> mesh, BlackMagic::AssetPointer<BlackMagic::Material> material) {};

		virtual ~Scene();
	};
}