#pragma once

#include <map>
#include <vector>

#include "StringManip.h"
#include "Mesh.h"
#include "Scene.h"

template<class T>
struct Asset
{
	typename T::Handle handle;
	std::string path;
	std::string name;
};

// TODO
// better tracking data to reduce loops and lookups would be great
template<class T>
struct Tracker
{
	std::map<std::string, size_t> fullPaths;
	std::map<typename T::Handle, size_t> handles;
	std::vector<Asset<T>> assets;
};

struct AssetTrackers :
	Tracker<MeshData>,
	Tracker<SceneData>
{};

struct DefaultAssets :
	MeshData::Handle,
	SceneData::Handle
{};

class AssetManager : public Singleton<AssetManager>
{
public:
	AssetManager();

	template<class T>
	void SetDefault(typename T::Handle handle); // make private once loading is in, or remove

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
	Asset<T>& TrackAsset(typename T::Handle handle, std::string fullPath);
	template<class T>
	void StopTrackingAsset(typename T::Handle handle);

	bool IsReady();
	bool CreateProject(std::string folder);
	bool LoadProject(std::string folder);
	void SaveProject();

	DefaultAssets defaults;

private:
	AssetTrackers trackers;
	bool ready;
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
Asset<T>& AssetManager::TrackAsset(typename T::Handle handle, std::string fullPath)
{
	Tracker<T>& tracker = trackers;
	tracker.fullPaths[fullPath] = tracker.assets.size();
	tracker.handles[handle] = tracker.assets.size();

	Asset<T> asset;
	asset.handle = handle;
	asset.path = fullPath;
	asset.name = StringManip::FileName(fullPath); // TODO manage better later? IE no duplicates

	tracker.assets.push_back(asset);
	return tracker.assets.back();
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
