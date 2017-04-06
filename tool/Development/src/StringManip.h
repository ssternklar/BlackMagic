#pragma once

#include <string>
#include <vector>

using namespace std;

namespace StringManip
{
	string Trim(const string& str, const string& whitespace = " \t");
	string Reduce(const string& str, const string& fill = " ", const string& whitespace = " \t");
	vector<string> &Split(const string &s, char delim, vector<string> &elems);
	vector<string> Split(const string &s, char delim);
	string ReplaceAll(string& source, const string& from, const string& to);
	string FileExtension(string path);
	string FileName(string path);
	string FilePath(string path);
}