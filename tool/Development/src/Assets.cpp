#include <algorithm>
#include <iterator>

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
	FileUtil::CreateDirectoryRecursive(MeshData::Instance().root);
	FileUtil::CreateDirectoryRecursive(TextureData::Instance().root);
	FileUtil::CreateDirectoryRecursive(SceneData::Instance().root);
	FileUtil::CreateDirectoryRecursive(VertexShaderData::Instance().root);
	FileUtil::CreateDirectoryRecursive(MaterialData::Instance().root);
	FileUtil::CreateDirectoryRecursive(MiscData::Instance().root);

	FILE* projFile;
	fopen_s(&projFile, "churo.proj", "wb");
	if (!projFile)
	{
		printf("failed to create project: could not create churo.proj\n");
		return false;
	}

	// metadata
	Internal::Proj::Meta meta = {};
	meta.nextUID = 5;
	meta.defaultMeshUID = 0;
	meta.defaultTextureUID = 1;
	meta.defaultVertexShaderUID = 2;
	meta.defaultPixelShaderUID = 3;
	meta.defaultMaterialUID = 4;
	meta.numMeshes = 1;
	meta.numTextures = 1;
	meta.numVertexShaders = 1;
	meta.numPixelShaders = 1;
	meta.numMaterials = 1;
	meta.numMisc = 0;
	meta.numScenes = 0;
	fwrite(&meta.nextUID, sizeof(Internal::Proj::Meta), 1, projFile);

	// default assets
	const char* defaultName = "default";
	const char* defaultMeshPath = "assets/defaults/Mesh.obj";
	const char* defaultTexturePath = "assets/defaults/Texture.png";
	const char* defaultVertexShaderPath = "assets/defaults/Vertex.hlsl";
	const char* defaultPixelShaderPath = "assets/defaults/Pixel.hlsl";
	const char* defaultMaterialPath = "assets/defaults/Material.mat";

	fwrite(&meta.defaultMeshUID, sizeof(Internal::Proj::Meta::defaultMeshUID), 1, projFile);
	fwrite(defaultMeshPath, strlen(defaultMeshPath) + 1, 1, projFile);
	fwrite(defaultName, strlen(defaultName) + 1, 1, projFile);

	fwrite(&meta.defaultTextureUID, sizeof(Internal::Proj::Meta::defaultTextureUID), 1, projFile);
	fwrite(defaultTexturePath, strlen(defaultTexturePath) + 1, 1, projFile);
	fwrite(defaultName, strlen(defaultName) + 1, 1, projFile);

	fwrite(&meta.defaultVertexShaderUID, sizeof(Internal::Proj::Meta::defaultVertexShaderUID), 1, projFile);
	fwrite(defaultVertexShaderPath, strlen(defaultVertexShaderPath) + 1, 1, projFile);
	fwrite(defaultName, strlen(defaultName) + 1, 1, projFile);

	fwrite(&meta.defaultPixelShaderUID, sizeof(Internal::Proj::Meta::defaultPixelShaderUID), 1, projFile);
	fwrite(defaultPixelShaderPath, strlen(defaultPixelShaderPath) + 1, 1, projFile);
	fwrite(defaultName, strlen(defaultName) + 1, 1, projFile);

	fwrite(&meta.defaultMaterialUID, sizeof(Internal::Proj::Meta::defaultMaterialUID), 1, projFile);
	fwrite(defaultMaterialPath, strlen(defaultMaterialPath) + 1, 1, projFile);
	fwrite(defaultName, strlen(defaultName) + 1, 1, projFile);

	// camera
	float origin[7] = {};
	origin[6] = 1.0f;
	fwrite(&origin, sizeof(float), 7, projFile);

	fclose(projFile);

	// write default files to disk
	FileUtil::WriteResourceToDisk(IDR_MESH1, "mesh", defaultMeshPath);
	FileUtil::WriteResourceToDisk(IDB_PNG1, "png", defaultTexturePath);
	FileUtil::WriteResourceToDisk(IDR_SHADER1, "shader", defaultVertexShaderPath);
	FileUtil::WriteResourceToDisk(IDR_SHADER2, "shader", defaultPixelShaderPath);
	FileUtil::WriteResourceToDisk(IDR_MATERIAL1, "material", defaultMaterialPath);

	// this does not work because graphics is live before a project exists, or can even load
	// write engine files to disk
	//FileUtil::WriteResourceToDisk(IDR_MESH2, "mesh", "engine/skybox.obj");
	//FileUtil::WriteResourceToDisk(IDR_CUBEMAP1, "cubemap", "engine/park_skybox_env.dds");
	//FileUtil::WriteResourceToDisk(IDR_CUBEMAP2, "cubemap", "engine/park_skybox_radiance.dds");
	//FileUtil::WriteResourceToDisk(IDR_CUBEMAP3, "cubemap", "engine/park_skybox_irradiance.dds");
	//FileUtil::WriteResourceToDisk(IDB_PNG2, "png", "engine/cosLUT.png");
	//FileUtil::WriteResourceToDisk(IDR_SHADER3, "shader", "engine/QuadVS.hlsl");
	//FileUtil::WriteResourceToDisk(IDR_SHADER4, "shader", "engine/LightPassPS.hlsl");
	//FileUtil::WriteResourceToDisk(IDR_SHADER5, "shader", "engine/ShadowMapVS.hlsl");
	//FileUtil::WriteResourceToDisk(IDR_SHADER6, "shader", "engine/SkyboxVS.hlsl");
	//FileUtil::WriteResourceToDisk(IDR_SHADER7, "shader", "engine/SkyboxPS.hlsl");
	//FileUtil::WriteResourceToDisk(IDR_SHADER8, "shader", "engine/FXAA_VS.hlsl");
	//FileUtil::WriteResourceToDisk(IDR_SHADER9, "shader", "engine/FXAA_PS.hlsl");
	//FileUtil::WriteResourceToDisk(IDR_SHADER10, "shader", "engine/FinalMerge.hlsl");

	LoadProject(folder);

	return true;
}

// TODO if i add loading mid-usage of the tool, track which assets are not used and revoke them
bool AssetManager::LoadProject(std::string folder)
{
	if (FileUtil::IsFolderEmpty(folder))
		return false;

	SetCurrentDirectoryA(folder.c_str());

	FILE* projFile;
	fopen_s(&projFile, "churo.proj", "rb");
	if (!projFile)
	{
		printf("failed to load project: could not open churo.proj\n");
		return false;
	}

	// load metadata
	Internal::Proj::Meta meta;
	fread_s(&meta.nextUID, sizeof(Internal::Proj::Meta), sizeof(Internal::Proj::Meta), 1, projFile);

	nextUID = meta.nextUID;

	// load assets
	Asset<MeshData> meshAsset = {};
	for (size_t i = 0; i < meta.numMeshes; ++i)
	{
		fread_s(&meshAsset.uID, sizeof(Internal::Proj::Asset::uID), sizeof(Internal::Proj::Asset::uID), 1, projFile);
		meshAsset.path = FileUtil::GetStringInFile(projFile);
		meshAsset.name = FileUtil::GetStringInFile(projFile);
		meshAsset.handle = MeshData::Instance().Load(meshAsset.path);
		AddAsset(meshAsset);

		if (meshAsset.uID == meta.defaultMeshUID)
			SetDefault<MeshData>(meshAsset.handle);
	}

	Asset<TextureData> textureAsset = {};
	for (size_t i = 0; i < meta.numTextures; ++i)
	{
		fread_s(&textureAsset.uID, sizeof(Internal::Proj::Asset::uID), sizeof(Internal::Proj::Asset::uID), 1, projFile);
		textureAsset.path = FileUtil::GetStringInFile(projFile);
		textureAsset.name = FileUtil::GetStringInFile(projFile);
		textureAsset.handle = TextureData::Instance().Load(textureAsset.path, TextureDesc::Type::FLAT, TextureDesc::Usage::READ);
		AddAsset(textureAsset);

		if (textureAsset.uID == meta.defaultTextureUID)
			SetDefault<TextureData>(textureAsset.handle);
	}

	Asset<VertexShaderData> vertexShaderAsset;
	for (size_t i = 0; i < meta.numVertexShaders; ++i)
	{
		fread_s(&vertexShaderAsset.uID, sizeof(Internal::Proj::Asset::uID), sizeof(Internal::Proj::Asset::uID), 1, projFile);
		vertexShaderAsset.path = FileUtil::GetStringInFile(projFile);
		vertexShaderAsset.name = FileUtil::GetStringInFile(projFile);
		vertexShaderAsset.handle = VertexShaderData::Instance().Load(vertexShaderAsset.path);
		AddAsset(vertexShaderAsset);

		if (vertexShaderAsset.uID == meta.defaultVertexShaderUID)
			SetDefault<VertexShaderData>(vertexShaderAsset.handle);
	}

	Asset<PixelShaderData> pixelShaderAsset;
	for (size_t i = 0; i < meta.numPixelShaders; ++i)
	{
		fread_s(&pixelShaderAsset.uID, sizeof(Internal::Proj::Asset::uID), sizeof(Internal::Proj::Asset::uID), 1, projFile);
		pixelShaderAsset.path = FileUtil::GetStringInFile(projFile);
		pixelShaderAsset.name = FileUtil::GetStringInFile(projFile);
		pixelShaderAsset.handle = PixelShaderData::Instance().Load(pixelShaderAsset.path);
		AddAsset(pixelShaderAsset);

		if (pixelShaderAsset.uID == meta.defaultPixelShaderUID)
			SetDefault<PixelShaderData>(pixelShaderAsset.handle);
	}

	Asset<MaterialData> materialAsset;
	for (size_t i = 0; i < meta.numMaterials; ++i)
	{
		fread_s(&materialAsset.uID, sizeof(Internal::Proj::Asset::uID), sizeof(Internal::Proj::Asset::uID), 1, projFile);
		materialAsset.path = FileUtil::GetStringInFile(projFile);
		materialAsset.name = FileUtil::GetStringInFile(projFile);
		materialAsset.handle = MaterialData::Instance().Load(materialAsset.path);
		AddAsset(materialAsset);

		if (materialAsset.uID == meta.defaultMaterialUID)
			SetDefault<MaterialData>(materialAsset.handle);
	}

	Asset<MiscData> miscAsset;
	for (size_t i = 0; i < meta.numMisc; ++i)
	{
		fread_s(&miscAsset.uID, sizeof(Internal::Proj::Asset::uID), sizeof(Internal::Proj::Asset::uID), 1, projFile);
		miscAsset.path = FileUtil::GetStringInFile(projFile);
		miscAsset.name = FileUtil::GetStringInFile(projFile);
		miscAsset.handle = MiscData::Instance().Load(miscAsset.path);
		AddAsset(miscAsset);
	}

	Asset<SceneData> sceneAsset;
	for (size_t i = 0; i < meta.numScenes; ++i)
	{
		fread_s(&sceneAsset.uID, sizeof(Internal::Proj::Asset::uID), sizeof(Internal::Proj::Asset::uID), 1, projFile);
		sceneAsset.path = FileUtil::GetStringInFile(projFile);
		sceneAsset.name = FileUtil::GetStringInFile(projFile);
		sceneAsset.handle = SceneData::Instance().Load(sceneAsset.path);
		AddAsset(sceneAsset);
	}

	// load scene config
	size_t sceneIndex;
	for (size_t i = 0; i < meta.numScenes; ++i)
	{
		fread_s(&sceneIndex, sizeof(size_t), sizeof(size_t), 1, projFile);
		SceneData::Instance().sceneExportConfig.push_back(GetAsset<SceneData>(sceneIndex).handle);
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
	Tracker<TextureData>& textureTracker = trackers;
	Tracker<SceneData>& sceneTracker = trackers;
	Tracker<VertexShaderData> vertexShaderTracker = trackers;
	Tracker<PixelShaderData> pixelShaderTracker = trackers;
	Tracker<MaterialData> materialTracker = trackers;
	Tracker<MiscData> miscTracker = trackers;

	MeshData::Handle& defaultMesh = defaults;
	TextureData::Handle& defaultTexture = defaults;
	VertexShaderData::Handle defaultVertexShader = defaults;
	PixelShaderData::Handle defaultPixelShader = defaults;
	MaterialData::Handle defaultMaterial = defaults;

	// save metadata
	Internal::Proj::Meta meta = {
		nextUID,
		GetAsset<MeshData>(defaultMesh).uID,
		GetAsset<TextureData>(defaultTexture).uID,
		GetAsset<VertexShaderData>(defaultVertexShader).uID,
		GetAsset<PixelShaderData>(defaultPixelShader).uID,
		GetAsset<MaterialData>(defaultMaterial).uID,
		meshTracker.assets.size(),
		textureTracker.assets.size(),
		vertexShaderTracker.assets.size(),
		pixelShaderTracker.assets.size(),
		materialTracker.assets.size(),
		miscTracker.assets.size(),
		sceneTracker.assets.size()
	};
	fwrite(&meta.nextUID, sizeof(Internal::Proj::Meta), 1, projFile);

	// save assets
	for (size_t i = 0; i < meta.numMeshes; ++i)
	{
		fwrite(&meshTracker.assets[i].uID, sizeof(Internal::Proj::Asset::uID), 1, projFile);
		fwrite(meshTracker.assets[i].path.c_str(), meshTracker.assets[i].path.length() + 1, 1, projFile);
		fwrite(meshTracker.assets[i].name.c_str(), meshTracker.assets[i].name.length() + 1, 1, projFile);
	}

	for (size_t i = 0; i < meta.numTextures; ++i)
	{
		fwrite(&textureTracker.assets[i].uID, sizeof(Internal::Proj::Asset::uID), 1, projFile);
		fwrite(textureTracker.assets[i].path.c_str(), textureTracker.assets[i].path.length() + 1, 1, projFile);
		fwrite(textureTracker.assets[i].name.c_str(), textureTracker.assets[i].name.length() + 1, 1, projFile);
	}

	for (size_t i = 0; i < meta.numVertexShaders; ++i)
	{
		fwrite(&vertexShaderTracker.assets[i].uID, sizeof(Internal::Proj::Asset::uID), 1, projFile);
		fwrite(vertexShaderTracker.assets[i].path.c_str(), vertexShaderTracker.assets[i].path.length() + 1, 1, projFile);
		fwrite(vertexShaderTracker.assets[i].name.c_str(), vertexShaderTracker.assets[i].name.length() + 1, 1, projFile);
	}

	for (size_t i = 0; i < meta.numPixelShaders; ++i)
	{
		fwrite(&pixelShaderTracker.assets[i].uID, sizeof(Internal::Proj::Asset::uID), 1, projFile);
		fwrite(pixelShaderTracker.assets[i].path.c_str(), pixelShaderTracker.assets[i].path.length() + 1, 1, projFile);
		fwrite(pixelShaderTracker.assets[i].name.c_str(), pixelShaderTracker.assets[i].name.length() + 1, 1, projFile);
	}

	for (size_t i = 0; i < meta.numMaterials; ++i)
	{
		MaterialData::Instance().Save(materialTracker.assets[i].handle);
		fwrite(&materialTracker.assets[i].uID, sizeof(Internal::Proj::Asset::uID), 1, projFile);
		fwrite(materialTracker.assets[i].path.c_str(), materialTracker.assets[i].path.length() + 1, 1, projFile);
		fwrite(materialTracker.assets[i].name.c_str(), materialTracker.assets[i].name.length() + 1, 1, projFile);
	}

	for (size_t i = 0; i < meta.numMisc; ++i)
	{
		fwrite(&miscTracker.assets[i].uID, sizeof(Internal::Proj::Asset::uID), 1, projFile);
		fwrite(miscTracker.assets[i].path.c_str(), miscTracker.assets[i].path.length() + 1, 1, projFile);
		fwrite(miscTracker.assets[i].name.c_str(), miscTracker.assets[i].name.length() + 1, 1, projFile);
	}

	for (size_t i = 0; i < meta.numScenes; ++i)
	{
		SceneData::Instance().Save(sceneTracker.assets[i].handle);
		fwrite(&sceneTracker.assets[i].uID, sizeof(Internal::Proj::Asset::uID), 1, projFile);
		fwrite(sceneTracker.assets[i].path.c_str(), sceneTracker.assets[i].path.length() + 1, 1, projFile);
		fwrite(sceneTracker.assets[i].name.c_str(), sceneTracker.assets[i].name.length() + 1, 1, projFile);
	}

	// save scene config
	size_t sceneIndex;
	for (size_t i = 0; i < meta.numScenes; ++i)
	{
		sceneIndex = GetIndex<SceneData>(SceneData::Instance().sceneExportConfig[i]);
		fwrite(&sceneIndex, sizeof(size_t), 1, projFile);
	}

	// save camera
	fwrite(&Camera::Instance().transform->pos, sizeof(float), 3, projFile);
	fwrite(&Camera::Instance().transform->rot, sizeof(float), 4, projFile);

	fclose(projFile);
}

bool AssetManager::Export(std::string name, bool force)
{
	string exportFolder = "exports/" + name + "/";
	FileUtil::CreateDirectoryRecursive(exportFolder);

	if (!force && !FileUtil::IsFolderEmpty(exportFolder))
		return false;

	if (force)
		FileUtil::DeleteDirectory(exportFolder);

	// scene data
	vector<SceneData::Handle> scenes = SceneData::Instance().sceneExportConfig;
	vector<Asset<SceneData>> usedSceneAssets;
	vector<uint16_t> uIDs;

	// file path data
	vector<size_t> filePathIndices;
	string filePathBlob = "";
	string filePath;

	size_t i;

	// scene scan data
	Asset<SceneData> sceneAsset;
	vector<uint16_t> tempUIDs;

	// scan the scenes and produce UIDs from them
	for (i = 0; i < scenes.size(); ++i)
	{
		if (!scenes[i]->willExport)
			continue;

		sceneAsset = AssetManager::Instance().GetAsset<SceneData>(scenes[i]);

		filePathIndices.resize(uIDs.size() + 1);
		filePathIndices[uIDs.size()] = filePathBlob.size();
		filePath = StringManip::ReplaceAll(sceneAsset.path, "assets/", "");
		filePathBlob += filePath + '\0';

		uIDs.push_back((uint16_t)sceneAsset.uID);
		tempUIDs = SceneData::Instance().Export(exportFolder + filePath, sceneAsset.handle);
		std::remove_copy_if(tempUIDs.begin(), tempUIDs.end(), back_inserter(uIDs), Contained(uIDs));

		usedSceneAssets.push_back(sceneAsset);
	}

	filePathIndices.resize(uIDs.size());

	// gather up the trackers
	Tracker<MeshData>& meshTracker = trackers;
	Tracker<TextureData>& textureTracker = trackers;
	Tracker<VertexShaderData> vertexShaderTracker = trackers;
	Tracker<PixelShaderData> pixelShaderTracker = trackers;
	Tracker<MaterialData> materialTracker = trackers;
	Tracker<MiscData> miscTracker = trackers;

	// export all assets and build file paths
	for (i = 0; i < meshTracker.assets.size(); ++i)
	{
		auto check = std::find(uIDs.begin(), uIDs.end(), meshTracker.assets[i].uID);
		if (check == uIDs.end())
			continue;

		filePathIndices[check - uIDs.begin()] = filePathBlob.size();
		filePath = StringManip::ReplaceAll(meshTracker.assets[i].path, "assets/", "");
		filePath = StringManip::ReplaceAll(filePath, "." + StringManip::FileExtension(filePath), ".bmmesh");
		filePathBlob += filePath + '\0';

		MeshData::Instance().Export(exportFolder + filePath, meshTracker.assets[i].handle);
	}

	for (i = 0; i < materialTracker.assets.size(); ++i)
	{
		auto check = std::find(uIDs.begin(), uIDs.end(), materialTracker.assets[i].uID);
		if (check == uIDs.end())
			continue;

		filePathIndices[check - uIDs.begin()] = filePathBlob.size();
		filePath = StringManip::ReplaceAll(materialTracker.assets[i].path, "assets/", "");
		filePath = StringManip::ReplaceAll(filePath, "." + StringManip::FileExtension(filePath), ".bmmat");
		filePathBlob += filePath + '\0';

		tempUIDs = MaterialData::Instance().Export(exportFolder + filePath, materialTracker.assets[i].handle);
		std::remove_copy_if(tempUIDs.begin(), tempUIDs.end(), back_inserter(uIDs), Contained(uIDs));
	}

	filePathIndices.resize(uIDs.size());

	for (i = 0; i < vertexShaderTracker.assets.size(); ++i)
	{
		auto check = std::find(uIDs.begin(), uIDs.end(), vertexShaderTracker.assets[i].uID);
		if (check == uIDs.end())
			continue;

		filePathIndices[check - uIDs.begin()] = filePathBlob.size();
		filePath = StringManip::ReplaceAll(vertexShaderTracker.assets[i].path, "assets/", "");
		filePathBlob += filePath + '\0';

		VertexShaderData::Instance().Export(exportFolder + filePath, vertexShaderTracker.assets[i].handle);
	}

	for (i = 0; i < pixelShaderTracker.assets.size(); ++i)
	{
		auto check = std::find(uIDs.begin(), uIDs.end(), pixelShaderTracker.assets[i].uID);
		if (check == uIDs.end())
			continue;

		filePathIndices[check - uIDs.begin()] = filePathBlob.size();
		filePath = StringManip::ReplaceAll(pixelShaderTracker.assets[i].path, "assets/", "");
		filePathBlob += filePath + '\0';

		PixelShaderData::Instance().Export(exportFolder + filePath, pixelShaderTracker.assets[i].handle);
	}

	for (i = 0; i < textureTracker.assets.size(); ++i)
	{
		auto check = std::find(uIDs.begin(), uIDs.end(), textureTracker.assets[i].uID);
		if (check == uIDs.end())
			continue;

		filePathIndices[check - uIDs.begin()] = filePathBlob.size();
		filePath = StringManip::ReplaceAll(textureTracker.assets[i].path, "assets/", "");
		filePathBlob += filePath + '\0';

		TextureData::Instance().Export(exportFolder + filePath, textureTracker.assets[i].handle);
	}

	for (i = 0; i < miscTracker.assets.size(); ++i)
	{
		auto check = std::find(uIDs.begin(), uIDs.end(), miscTracker.assets[i].uID);
		if (check == uIDs.end())
			continue;

		filePathIndices[check - uIDs.begin()] = filePathBlob.size();
		filePath = StringManip::ReplaceAll(miscTracker.assets[i].path, "assets/", "");
		filePathBlob += filePath + '\0';

		MiscData::Instance().Export(exportFolder + filePath, miscTracker.assets[i].handle);
	}

	// write manifest
	filePath = exportFolder + "manifest.bm";

	FILE* manifestFile;
	fopen_s(&manifestFile, filePath.c_str(), "wb");
	if (!manifestFile)
	{
		printf("Failed to write manifest file '%s'", filePath.c_str());
		return false;
	}

	Export::Manifest::File fileData;

	// write meta to manifest
	fileData.pathBlockSize = (uint16_t)filePathBlob.size();
	fileData.numAssets = (uint16_t)uIDs.size();
	fileData.numScenes = (uint16_t)usedSceneAssets.size();

	fwrite(&fileData.pathBlockSize, sizeof(uint16_t), 3, manifestFile);

	// write scene uIDs to manifest
	for (i = 0; i < usedSceneAssets.size(); ++i)
		fwrite(&usedSceneAssets[i].uID, sizeof(Export::Manifest::Asset::uID), 1, manifestFile);

	// write assets to manifest
	Export::Manifest::Asset asset = {};
	struct _stat statBuf;

	for (i = 0; i < fileData.numAssets; ++i)
	{
		asset.uID = uIDs[i];
		asset.filePathIndex = (uint16_t)filePathIndices[i];

		filePath = filePathBlob.substr(filePathIndices[i]);
		_stat((exportFolder + filePath).c_str(), &statBuf);
		asset.fileSize = statBuf.st_size;

		fwrite(&asset.uID, sizeof(Export::Manifest::Asset), 1, manifestFile);
	}

	// write file path blob to manifest
	fwrite(filePathBlob.c_str(), sizeof(char), filePathBlob.size(), manifestFile);

	fclose(manifestFile);

	return true;
}
