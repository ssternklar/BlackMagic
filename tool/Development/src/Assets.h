#pragma once

#include <map>
#include <vector>

#include "Patterns.h"
#include "Mesh.h"

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
	std::map<std::string, size_t> paths;
	std::vector<Asset<T>> assets;
};

struct AssetTrackers :
	Tracker<MeshData>
{};

struct DefaultAssets :
	MeshData::Handle
{};

class AssetManager : public Singleton<AssetManager>
{
public:
	template<class T>
	void SetDefault(typename T::Handle handle); // make private once loading is in, or remove

	template<class T>
	Asset<T>& GetAsset(size_t index);
	template<class T>
	size_t GetAssetCount();
	template<class T>
	size_t GetIndex(typename T::Handle handle);


	template<class T>
	typename T::Handle GetHandle(std::string path);

	template<class T>
	void TrackAsset(Asset<T> asset);
	template<class T>
	void StopTrackingAsset(typename T::Handle handle);

	//void CreateProject(std::string folder); // given an empty folder, populates it
	//void LoadProject(std::string folder); // loads all assets in the manifest save file (auto-called after CreateProject)

	DefaultAssets defaults;

private:
	AssetTrackers trackers;
};

template<class T>
void AssetManager::SetDefault(typename T::Handle handle)
{
	T::Handle &h = defaults;
	h = handle;
}

template<class T>
Asset<T>& AssetManager::GetAsset(size_t index)
{
	Tracker<T> &tracker = trackers;
	return tracker.assets[index];
}

template<class T>
size_t AssetManager::GetAssetCount()
{
	Tracker<T> &tracker = trackers;
	return tracker.assets.size();
}

template<class T>
size_t AssetManager::GetIndex(typename T::Handle handle)
{
	Tracker<T> &tracker = trackers;
	size_t i = 0;

	for (; i < tracker.assets.size(); ++i)
		if (tracker.assets[i].handle == handle)
			break;

	return i;
}

template<class T>
typename T::Handle AssetManager::GetHandle(std::string path)
{
	Tracker<T> &tracker = trackers;
	T::Handle handle;

	auto check = tracker.paths.find(path);
	if (check != tracker.paths.end())
		handle = tracker.assets[check->second].handle;

	return handle;
}

template<class T>
void AssetManager::TrackAsset(Asset<T> asset)
{
	Tracker<T> &tracker = trackers;
	tracker.paths[asset.path] = tracker.assets.size();
	tracker.assets.push_back(asset);
}

template<class T>
void AssetManager::StopTrackingAsset(typename T::Handle handle)
{
	Tracker<T> &tracker = trackers;

	for (size_t i = 0; i < tracker.assets.size(); ++i)
		if (tracker.assets[i].handle == handle)
		{
			tracker.paths.erase(tracker.paths.find(tracker.assets[i].path));
			tracker.assets.erase(tracker.assets.begin() + i);
			break;
		}
}
