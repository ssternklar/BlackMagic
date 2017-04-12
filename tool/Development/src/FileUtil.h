#pragma once

#include "StringManip.h"

namespace FileUtil
{
	string GetStringInFile(FILE* file);
	int CopyFileViaPaths(string from, string to);
	int DeleteDirectory(const string &refcstrRootDirectory, bool bDeleteSubdirectories = true);
	int CopyBytes(FILE* fromFile, FILE* toFile, size_t count);
	bool DoesfileExist(string path);
	void CreateDirectoryRecursive(string filePath);
	string BrowseFolder();
	string getexepath();
	bool IsFolderEmpty(string folder);
	bool WriteResourceToDisk(const int resourceName, const char* resourceType, const char* filePath);
}
