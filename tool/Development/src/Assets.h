#pragma once

#include <map>
#include <vector>

#include "StringManip.h"
#include "Mesh.h"
#include "Scene.h"
#include "Texture.h"
#include "Shader.h"
#include "Material.h"

#define DX_RELEASE(ptr) if(ptr) { ptr->Release(); }

// http://stackoverflow.com/questions/5529067/c-appending-one-vector-to-another-with-removal-of-duplicates

struct Contained
{
	const vector<uint16_t>& _sequence;
	Contained(const vector<uint16_t> &vec) : _sequence(vec) {}
	bool operator()(int i) const
	{
		return _sequence.end() != std::find(_sequence.begin(), _sequence.end(), i);
	}
};

template<class T>
struct Asset
{
	typename T::Handle handle;
	size_t uID;
	std::string path;
	std::string name;
	bool operator==(const Asset<T>& other) const { return uID == other.uID; }
	bool operator!=(const Asset<T>& other) const { return uID != other.uID; }
	bool operator<(const Asset<T>& other) const { return uID < other.uID; }
};

template<class T>
struct Tracker
{
	std::map<std::string, size_t> fullPaths;
	std::map<typename T::Handle, size_t> handles;
	std::vector<Asset<T>> assets;
};

struct AssetTrackers :
	Tracker<MeshData>,
	Tracker<SceneData>,
	Tracker<TextureData>,
	Tracker<VertexShaderData>,
	Tracker<PixelShaderData>,
	Tracker<MaterialData>
{};

struct DefaultAssets :
	MeshData::Handle,
	TextureData::Handle,
	VertexShaderData::Handle,
	PixelShaderData::Handle,
	MaterialData::Handle
{};

class AssetManager : public Singleton<AssetManager>
{
public:
	AssetManager();

	template<class T>
	Asset<T>& GetAsset(size_t index);
	template<class T>
	Asset<T>& GetAsset(typename T::Handle handle);
	template<class T>
	size_t GetAssetCount();
	template<class T>
	size_t GetIndex(typename T::Handle handle);
	template<class T>
	typename T::Handle GetHandle(std::string fullPath);

	template<class T>
	void TrackAsset(typename T::Handle handle, std::string fullPath);
	template<class T>
	void StopTrackingAsset(typename T::Handle handle);

	bool IsReady();
	bool CreateProject(std::string folder);
	bool LoadProject(std::string folder);
	void SaveProject();
	bool Export(std::string name, bool force);

	DefaultAssets defaults;

private:
	template<class T>
	void SetDefault(typename T::Handle handle);
	template<class T>
	void AddAsset(Asset<T> asset);

	AssetTrackers trackers;
	bool ready;
	size_t nextUID;
};

template<class T>
void AssetManager::SetDefault(typename T::Handle handle)
{
	T::Handle& h = defaults;
	h = handle;
}

template<class T>
Asset<T>& AssetManager::GetAsset(size_t index)
{
	Tracker<T>& tracker = trackers;
	return tracker.assets[index];
}

template<class T>
Asset<T>& AssetManager::GetAsset(typename T::Handle handle)
{
	Tracker<T>& tracker = trackers;
	return tracker.assets[tracker.handles[handle]];
}

template<class T>
size_t AssetManager::GetAssetCount()
{
	Tracker<T>& tracker = trackers;
	return tracker.assets.size();
}

template<class T>
size_t AssetManager::GetIndex(typename T::Handle handle)
{
	Tracker<T>& tracker = trackers;
	return tracker.handles[handle];
}

template<class T>
typename T::Handle AssetManager::GetHandle(std::string fullPath)
{
	Tracker<T>& tracker = trackers;
	T::Handle handle;

	auto check = tracker.fullPaths.find(fullPath);
	if (check != tracker.fullPaths.end())
		handle = tracker.assets[check->second].handle;

	return handle;
}

template<class T>
void AssetManager::TrackAsset(typename T::Handle handle, std::string fullPath)
{
	Tracker<T>& tracker = trackers;
	tracker.fullPaths[fullPath] = tracker.assets.size();
	tracker.handles[handle] = tracker.assets.size();

	Asset<T> asset;
	asset.handle = handle;
	asset.uID = nextUID++;
	asset.path = fullPath;
	asset.name = StringManip::FileName(fullPath); // TODO manage better later? IE no duplicates

	tracker.assets.push_back(asset);
}

template<class T>
void AssetManager::StopTrackingAsset(typename T::Handle handle)
{
	Tracker<T>& tracker = trackers;

	size_t index = tracker.handles[handle];

	tracker.fullPaths.erase(tracker.fullPaths.find(tracker.assets[index].path));
	tracker.handles.erase(tracker.handles.find(handle));
	tracker.assets.erase(tracker.assets.begin() + index);
}

template<class T>
void AssetManager::AddAsset(Asset<T> asset)
{
	Tracker<T>& tracker = trackers;
	tracker.fullPaths[asset.path] = tracker.assets.size();
	tracker.handles[asset.handle] = tracker.assets.size();
	tracker.assets.push_back(asset);
}
