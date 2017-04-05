#include "Assets.h"
#include "FileUtil.h"
#include "../resource.h"
#include "FileFormats.h"

AssetManager::AssetManager()
{
	ready = false;
}

bool AssetManager::IsReady()
{
	return ready;
}

bool AssetManager::CreateProject(std::string folder)
{
	if (!FileUtil::IsFolderEmpty(folder))
		return false;

	SetCurrentDirectoryA(folder.c_str());

	FileUtil::CreateDirectoryRecursive("assets/defaults/");
	FileUtil::CreateDirectoryRecursive("assets/models/");
	FileUtil::CreateDirectoryRecursive("assets/scenes/");

	FILE* projFile;
	fopen_s(&projFile, "churo.proj", "wb");
	if (!projFile)
	{
		printf("failed to create project: could not create churo.proj\n");
		return false;
	}

	// default paths
	const char* defaultMeshPath = "assets/defaults/Mesh.obj";
	fwrite(defaultMeshPath, strlen(defaultMeshPath) + 1, 1, projFile);

	// metadata
	size_t zero[2] = {};
	fwrite(&zero, sizeof(size_t), 2, projFile);

	// camera
	float origin[7] = {};
	origin[6] = 1.0f;
	fwrite(&origin, sizeof(float), 7, projFile);

	fclose(projFile);
	
	// write default files to disk
	FileUtil::WriteResourceToDisk(IDR_MESH1, "mesh", defaultMeshPath);

	LoadProject(folder);

	return true;
}

bool AssetManager::LoadProject(std::string folder)
{
	if (FileUtil::IsFolderEmpty(folder))
		return false;

	// TODO
	// if i add loading mid-usage of the tool, track which assets are not used and revoke them

	SetCurrentDirectoryA(folder.c_str());

	FILE* projFile;
	fopen_s(&projFile, "churo.proj", "rb");
	if (!projFile)
	{
		printf("failed to load project: could not open churo.proj\n");
		return false;
	}

	// load defaults
	string path = FileUtil::GetStringInFile(projFile);
	MeshData::Handle mesh = MeshData::Instance().LoadMesh(path);
	SetDefault<MeshData>(mesh);
	TrackAsset<MeshData>(mesh, path).name = "default";

	// load metadata
	size_t meshCount, sceneCount;
	fread_s(&meshCount, sizeof(size_t), sizeof(size_t), 1, projFile);
	fread_s(&sceneCount, sizeof(size_t), sizeof(size_t), 1, projFile);

	// load assets
	for (size_t i = 0; i < meshCount; ++i)
	{
		path = FileUtil::GetStringInFile(projFile);
		mesh = MeshData::Instance().LoadMesh(path);
		TrackAsset<MeshData>(mesh, path).name = FileUtil::GetStringInFile(projFile);
	}

	SceneData::Handle scene;
	for (size_t i = 0; i < sceneCount; ++i)
	{
		path = FileUtil::GetStringInFile(projFile);
		scene = SceneData::Instance().LoadScene(path);
		TrackAsset<SceneData>(scene, path).name = FileUtil::GetStringInFile(projFile);
	}

	// load camera
	fread_s(&Camera::Instance().transform->pos, sizeof(DirectX::XMFLOAT3), sizeof(float), 3, projFile);
	fread_s(&Camera::Instance().transform->rot, sizeof(DirectX::XMFLOAT4), sizeof(float), 4, projFile);

	fclose(projFile);

	ready = true;

	return true;
}

void AssetManager::SaveProject()
{
	FILE* projFile;
	fopen_s(&projFile, "churo.proj", "wb");
	if (!projFile)
	{
		printf("failed to save project: could not write to churo.proj\n");
		return;
	}

	// gather up the trackers and defaults
	Tracker<MeshData>& meshTracker = trackers;
	Tracker<SceneData>& sceneTracker = trackers;

	MeshData::Handle& defaultMesh = defaults;
	Asset<MeshData>& defaultMeshAsset = GetAsset<MeshData>(defaultMesh);

	// save defaults
	fwrite(defaultMeshAsset.path.c_str(), defaultMeshAsset.path.length() + 1, 1, projFile);
	
	// save metadata
	size_t meshCount = meshTracker.assets.size() - 1;
	fwrite(&meshCount, sizeof(size_t), 1, projFile);

	size_t sceneCount = sceneTracker.assets.size();
	fwrite(&sceneCount, sizeof(size_t), 1, projFile);

	// save assets
	for (size_t i = 0; i < meshTracker.assets.size(); ++i)
	{
		if (meshTracker.assets[i].handle != defaultMesh)
		{
			fwrite(meshTracker.assets[i].path.c_str(), meshTracker.assets[i].path.length() + 1, 1, projFile);
			fwrite(meshTracker.assets[i].name.c_str(), meshTracker.assets[i].name.length() + 1, 1, projFile);
		}
	}

	for (size_t i = 0; i < sceneCount; ++i)
	{
		SceneData::Instance().SaveScene(sceneTracker.assets[i].handle);
		fwrite(sceneTracker.assets[i].path.c_str(), sceneTracker.assets[i].path.length() + 1, 1, projFile);
		fwrite(sceneTracker.assets[i].name.c_str(), sceneTracker.assets[i].name.length() + 1, 1, projFile);
	}

	// save camera
	fwrite(&Camera::Instance().transform->pos, sizeof(float), 3, projFile);
	fwrite(&Camera::Instance().transform->rot, sizeof(float), 4, projFile);

	fclose(projFile);
}

// TODO make UIDs a part of the Asset struct, will require export rewrite too
bool AssetManager::Export(std::string name, bool force)
{
	string exportFolder = "exports/" + name + "/";
	FileUtil::CreateDirectoryRecursive(exportFolder);

	if (!force && !FileUtil::IsFolderEmpty(exportFolder))
		return false;

	if (force)
		FileUtil::DeleteDirectory(exportFolder);

	Tracker<SceneData>& sceneTracker = trackers;
	vector<Asset<MeshData>> usedMeshAssets;

	// find all used mesh assets
	size_t i, j;
	size_t sceneCount = sceneTracker.assets.size();
	size_t entityCount;
	SceneData::Handle scene;
	Asset<MeshData> meshAsset;
	for (i = 0; i < sceneCount; ++i)
	{
		scene = sceneTracker.assets[i].handle;
		entityCount = scene->entities.size();
		for (j = 0; j < entityCount; ++j)
		{
			meshAsset = GetAsset<MeshData>(scene->entities[j]->mesh);
			auto check = std::find(usedMeshAssets.begin(), usedMeshAssets.end(), meshAsset);
			if (check == usedMeshAssets.end())
				usedMeshAssets.push_back(meshAsset);
		}
	}

	// build manifest path blob
	map<MeshData::Handle, uint16_t> meshUIDs;
	vector<size_t> filePathIndexes;
	filePathIndexes.reserve(sceneCount + usedMeshAssets.size());
	string filePathBlob = "";
	string filePath;

	for (i = 0; i < sceneCount; ++i)
	{
		filePathIndexes.push_back(filePathBlob.size());
		filePath = StringManip::ReplaceAll(sceneTracker.assets[i].path, "assets/", exportFolder);
		filePathBlob += filePath + '\0';
	}

	for (i = 0; i < usedMeshAssets.size(); ++i)
	{
		meshAsset = usedMeshAssets[i];
		meshUIDs[meshAsset.handle] = (uint16_t)(sceneCount + i);

		filePathIndexes.push_back(filePathBlob.size());
		filePath = StringManip::ReplaceAll(meshAsset.path, "assets/", exportFolder);
		filePath = StringManip::ReplaceAll(filePath, StringManip::FileExtension(filePath), "mesh");
		filePathBlob += filePath + '\0';

		MeshData::Instance().Export(filePath, meshAsset.handle);
	}

	// TODO move to SceneData once UIDs are in Assets
	// scenes
	vector<uint16_t> sceneUIDs;
	uint16_t meshUID;
	Export::Scene::File sceneStruct = {};
	Export::Scene::Transform transform = {};

	for (i = 0; i < sceneCount; ++i)
	{
		// find used UIDs
		sceneUIDs.clear();
		scene = sceneTracker.assets[i].handle;
		entityCount = scene->entities.size();

		for (j = 0; j < entityCount; ++j)
		{
			auto check = meshUIDs.find(scene->entities[j]->mesh);
			if (check != meshUIDs.end())
				sceneUIDs.push_back(check->second);
		}

		// meta data
		sceneStruct.numAssets = (uint16_t)sceneUIDs.size();
		sceneStruct.numEntities = (uint16_t)entityCount;

		// create the scene file
		filePath = filePathBlob.substr(filePathIndexes[i]);
		FileUtil::CreateDirectoryRecursive(filePath.c_str());

		FILE* sceneFile;
		fopen_s(&sceneFile, filePath.c_str(), "wb");
		if (!sceneFile)
		{
			printf("Failed to write scene file '%s'", filePath.c_str());
			return false;
		}

		// write meta data
		fwrite(&sceneStruct.numAssets, sizeof(uint16_t), 1, sceneFile);
		fwrite(&sceneUIDs[0], sizeof(uint16_t), sceneUIDs.size(), sceneFile);
		fwrite(&sceneStruct.numEntities, sizeof(uint16_t), 1, sceneFile);

		// write entities
		for (j = 0; j < entityCount; ++j)
		{
			memcpy_s(&transform.pos[0], sizeof(float) * 3, &scene->entities[j]->transform->pos.x, sizeof(DirectX::XMFLOAT3));
			memcpy_s(&transform.rot[0], sizeof(float) * 4, &scene->entities[j]->transform->rot.x, sizeof(DirectX::XMFLOAT4));
			memcpy_s(&transform.scale, sizeof(float), &scene->entities[j]->transform->scale, sizeof(float));

			meshUID = meshUIDs[scene->entities[j]->mesh];

			fwrite(&transform.pos[0], sizeof(Export::Scene::Transform), 1, sceneFile);
			fwrite(&meshUID, sizeof(uint16_t), 1, sceneFile);
		}

		fclose(sceneFile);
	}

	// process file path blob
	filePathBlob = StringManip::ReplaceAll(filePathBlob, exportFolder, "");
	for (i = 1; i < filePathIndexes.size(); ++i)
		filePathIndexes[i] -= i * exportFolder.length();

	// write manifest
	filePath = exportFolder + "manifest.bm";

	FILE* manifestFile;
	fopen_s(&manifestFile, filePath.c_str(), "wb");
	if (!manifestFile)
	{
		printf("Failed to write manifest file '%s'", filePath.c_str());
		return false;
	}

	Export::Manifest::File manifest;

	// write meta to manifest
	manifest.pathBlockSize = (uint16_t)filePathBlob.size();
	manifest.numAssets = (uint16_t)filePathIndexes.size();

	fwrite(&manifest.pathBlockSize, sizeof(uint16_t), 2, manifestFile);

	// write assets to manifest
	Export::Manifest::Asset asset = {};
	struct _stat statBuf;

	for (i = 0; i < manifest.numAssets; ++i)
	{
		filePath = filePathBlob.substr(filePathIndexes[asset.uID]);
		asset.filePathIndex = (uint16_t)filePathIndexes[asset.uID];

		_stat((exportFolder + filePath).c_str(), &statBuf);
		asset.fileSize = (uint16_t)statBuf.st_size;

		fwrite(&asset.uID, sizeof(uint16_t), 3, manifestFile);

		asset.uID++;
	}

	// write file path blob to manifest
	fwrite(filePathBlob.c_str(), sizeof(char), filePathBlob.size(), manifestFile);

	fclose(manifestFile);

	return true;
}
