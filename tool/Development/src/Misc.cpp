#include "Misc.h"
#include "Assets.h"
#include "FileUtil.h"

MiscData::Handle MiscData::Get(std::string path)
{
	std::string fullPath = root + path;
	
	Handle h = AssetManager::Instance().GetHandle<MiscData>(fullPath);
	if (h.ptr())
		return h;

	h = Load(fullPath);

	if (!h.ptr())
		return h;

	AssetManager::Instance().TrackAsset<MiscData>(h, fullPath);

	return h;
}

void MiscData::Revoke(Handle handle)
{
	ProxyHandler::Revoke(handle);
}

void MiscData::Export(std::string path, Handle handle)
{
	FileUtil::CopyFileViaPaths(AssetManager::Instance().GetAsset<MiscData>(handle).path, path);
}

MiscData::Handle MiscData::Load(std::string fullPath)
{
	Handle h;

	if (FileUtil::DoesfileExist(fullPath))
	{
		h = ProxyHandler::Get();

		// not actually used
		h->dumb = 0;
	}
	
	return h;
}
