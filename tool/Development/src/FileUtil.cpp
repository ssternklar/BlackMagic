#include "FileUtil.h"

#define NOMINMAX

#include <Windows.h>
//#include <fstream>
#include <Shlwapi.h>
#include <shlobj.h>
//#include <iostream>
//#include <sstream>

namespace FileUtil
{
	string GetStringInFile(FILE* file)
	{
		string out = "";
		char c;
		do
		{
			c = fgetc(file);
			if (c != EOF && c != '\0') out += c;
		} while (c != EOF && c != '\0');
		return out;
	}

	int CopyFile(string from, string to)
	{
		CreateDirectoryRecursive(to);

		FILE* fromFile, *toFile;
		fopen_s(&fromFile, from.c_str(), "rb");
		fopen_s(&toFile, to.c_str(), "wb");
		if (!fromFile || !toFile)
			return EXIT_FAILURE;

		int b;
		do
		{
			b = fgetc(fromFile);
			if (b != EOF) fputc(b, toFile);
		} while (b != EOF);

		return EXIT_SUCCESS;
	}

	// http://stackoverflow.com/questions/734717/how-to-delete-a-folder-in-c
	int DeleteDirectoryFix(const string &refcstrRootDirectory,	bool bDeleteSubdirectories, size_t depth)
	{
		bool bSubdirectory = false; // Flag, indicating whether subdirectories have been found
		HANDLE hFile; // Handle to directory
		string strFilePath; // Filepath
		string strPattern; // Pattern
		WIN32_FIND_DATAA FileInformation; // File information

		strPattern = refcstrRootDirectory + "/*.*";
		hFile = FindFirstFileA(strPattern.c_str(), &FileInformation);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (FileInformation.cFileName[0] != '.')
				{
					strFilePath.erase();
					strFilePath = refcstrRootDirectory + "/" + FileInformation.cFileName;

					if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (bDeleteSubdirectories)
						{
							// Delete subdirectory
							int iRC = DeleteDirectoryFix(strFilePath, bDeleteSubdirectories, depth + 1);
							if (iRC)
								return iRC;
						}
						else
							bSubdirectory = true;
					}
					else
					{
						// Set file attributes
						if (!SetFileAttributesA(strFilePath.c_str(), FILE_ATTRIBUTE_NORMAL))
							return GetLastError();

						// Delete file
						if (!DeleteFileA(strFilePath.c_str()))
							return GetLastError();
					}
				}
			} while (FindNextFileA(hFile, &FileInformation));

			// Close handle
			FindClose(hFile);

			DWORD dwError = GetLastError();
			if (dwError != ERROR_NO_MORE_FILES)
				return dwError;
			else
			{
				if (!bSubdirectory && depth > 0)
				{
					// Set directory attributes
					if (!SetFileAttributesA(refcstrRootDirectory.c_str(), FILE_ATTRIBUTE_NORMAL))
						return GetLastError();

					// Delete directory
					if (!RemoveDirectoryA(refcstrRootDirectory.c_str()))
						return GetLastError();
				}
			}
		}

		return 0;
	}

	int DeleteDirectory(const string &refcstrRootDirectory, bool bDeleteSubdirectories)
	{
		return DeleteDirectoryFix(refcstrRootDirectory, bDeleteSubdirectories, 0);
	}

	int CopyBytes(FILE* fromFile, FILE* toFile, size_t count)
	{
		int b;
		if (count > 0)
			for (size_t i = 0; i < count; ++i)
			{
				b = fgetc(fromFile);
				if (b != EOF)
					fputc(b, toFile);
				else
					break;
			}
		else
			do
			{
				b = fgetc(fromFile);
				if (b != EOF) fputc(b, toFile);
			} while (b != EOF);

		return EXIT_SUCCESS;
	}

	bool DoesfileExist(string path)
	{
		if (path == "")
			return false;
		struct stat buffer;
		return !(stat(path.c_str(), &buffer) == -1);
	}

	void CreateDirectoryRecursive(string filePath)
	{
		filePath = filePath.substr(0, filePath.find_last_of("/"));

		if (filePath == "" || PathFileExistsA(filePath.c_str()))
			return;

		if (!CreateDirectoryA(filePath.c_str(), NULL))
		{
			CreateDirectoryRecursive(filePath);
			CreateDirectoryA(filePath.c_str(), NULL);
		}
	}

	// http://stackoverflow.com/questions/12034943/win32-select-directory-dialog-from-c-c

	static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
	{

		if (uMsg == BFFM_INITIALIZED)
		{
			string tmp = (const char *)lpData;
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		}

		return 0;
	}

	string BrowseFolder()
	{
		static string saved_path = "";

		if (saved_path.length() == 0)
			saved_path = getexepath();

		TCHAR path[MAX_PATH];

		const char * path_param = saved_path.c_str();

		BROWSEINFO bi = { 0 };
		bi.lpszTitle = ("Browse for folder...");
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = (LPARAM)path_param;

		LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

		if (pidl != 0)
		{
			//get the name of the folder and put it in path
			SHGetPathFromIDList(pidl, path);

			//free memory used
			IMalloc * imalloc = 0;
			if (SUCCEEDED(SHGetMalloc(&imalloc)))
			{
				imalloc->Free(pidl);
				imalloc->Release();
			}

			saved_path = path;
			return StringManip::ReplaceAll(saved_path,"\\", "/");
		}

		return "";
	}

	// http://stackoverflow.com/questions/143174/how-do-i-get-the-directory-that-a-program-is-running-from

	string getexepath()
	{
		char result[MAX_PATH];
		string path = string(result, GetModuleFileName(NULL, result, MAX_PATH));
		return StringManip::FilePath(path);
	}
}
