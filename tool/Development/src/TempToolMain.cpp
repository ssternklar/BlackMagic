#include <iostream>
#include <fstream>
#include <string>
#include <direct.h>
#include "FileFormats.h"
#include "FileUtil.h"
#include "Graphics.h"
#include "Mesh.h"

size_t FileSize(std::string name)
{
	std::wstring wname(name.length(), L' ');
	std::copy(name.begin(), name.end(), wname.begin());
	HANDLE hFile = CreateFile(name.c_str(), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1; // error condition, could call GetLastError to find out more

	LARGE_INTEGER size;
	if (!GetFileSizeEx(hFile, &size))
	{
		CloseHandle(hFile);
		return -1; // error condition, could call GetLastError to find out more
	}

	CloseHandle(hFile);
	return size.QuadPart;
}

std::string getPath(std::string fileName)
{
	return fileName.substr(0, fileName.find_last_of('/'));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Graphics::Instance().Init(hInstance, 800, 600);
	MeshData::Instance().Init(Graphics::Instance().GetDevice());

	//Change this for your manifest description
	std::string fileName = "C:\\Users\\Sam\\Documents\\Visual Studio 2015\\Projects\\Racing\\assets\\manifestDesc.txt";
	fileName = StringManip::ReplaceAll(fileName, "\\", "/");
	std::string path = getPath(fileName);
	//chdir(path.c_str());

	std::ifstream infile(fileName);

	std::string line;
	std::vector<Export::Manifest::Asset> assets;
	std::vector<std::string> strings;
	size_t uid = 0;
	size_t index = 0;
	while (infile.peek() != EOF)
	{
		std::getline(infile, line);
		std::string fullPath = path + line;
		Export::Manifest::Asset as;
		as.uID = uid;
		as.filePathIndex = index;
		if (line.find(".obj") != -1)
		{
			std::string newName = line.substr(0, line.size() - 4) + ".bmmesh";
			std::string newPath = path + newName;
			MeshData::Handle mesh = MeshData::Instance().LoadMesh(fullPath);
			MeshData::Instance().Export(newPath, mesh);
			fullPath = newPath;
			line = newName;
		}
		as.fileSize = FileSize(fullPath);
		strings.push_back(line);
		assets.push_back(as);
		index += line.size() + 1;
		uid++;
	}

	Export::Manifest::File file;
	file.numAssets = assets.size();
	file.pathBlockSize = index;
	std::ofstream out(path + "/manifest.bm", std::ios::binary);
	out.write((char*)&file, sizeof(Export::Manifest::File));
	out.write((char*)&assets[0], assets.size() * sizeof(Export::Manifest::Asset));
	
	char buf[5] = { '\0','\0' ,'\0' ,'\0' ,'\0' };
	
	for (int i = 0; i < strings.size(); i++)
	{
		out.write(strings[i].c_str(), strings[i].size());
		out.write(buf, 1);
	}
	out.flush();
	out.close();
}