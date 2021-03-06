#include "StringManip.h"

#include <sstream>
#include <Windows.h>

namespace StringManip
{
	// http://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string

	string Trim(const string& str, const string& whitespace)
	{
		const auto strBegin = str.find_first_not_of(whitespace);
		if (strBegin == string::npos)
			return ""; // no content

		const auto strEnd = str.find_last_not_of(whitespace);
		const auto strRange = strEnd - strBegin + 1;

		return str.substr(strBegin, strRange);
	}

	string Reduce(const string& str, const string& fill, const string& whitespace)
	{
		// trim first
		auto result = Trim(str, whitespace);

		// replace sub ranges
		auto beginSpace = result.find_first_of(whitespace);
		while (beginSpace != string::npos)
		{
			const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
			const auto range = endSpace - beginSpace;

			result.replace(beginSpace, range, fill);

			const auto newStart = beginSpace + fill.length();
			beginSpace = result.find_first_of(whitespace, newStart);
		}

		return result;
	}

	// http://stackoverflow.com/questions/236129/split-a-string-in-c

	vector<string> &Split(const string &s, char delim, vector<string> &elems)
	{
		stringstream ss(s);
		string item;
		while (getline(ss, item, delim))
			elems.push_back(item);
		return elems;
	}


	vector<string> Split(const string &s, char delim)
	{
		vector<string> elems;
		return Split(s, delim, elems);
	}

	// http://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string

	string ReplaceAll(std::string& source, const std::string& from, const std::string& to)
	{
		string str = source;
		if (!from.empty())
		{
			size_t start_pos = 0;
			while ((start_pos = str.find(from, start_pos)) != std::string::npos)
			{
				str.replace(start_pos, from.length(), to);
				start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
			}
		}
		return str;
	}

	string FileExtension(string path)
	{
		return path.substr(path.rfind('.') + 1);
	}

	string FileName(string path)
	{
		size_t start = path.rfind('/');
		return path.substr(start + 1, path.rfind('.') - start - 1);
	}

	string FilePath(string path)
	{
		char slash = '/';
		if (path.find('\\') != string::npos)
			slash = '\\';

		size_t index = path.rfind(slash);
		size_t index2;
		path = path.substr(0, index + 1);

		while ((index = path.find("..")) != string::npos)
		{
			index2 = path.rfind(slash, index - 2);
			path = path.erase(index2, index - index2 + 2);
		}

		return path;
	}

	// http://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte

	wstring utf8_decode(const string &str)
	{
		if (str.empty()) return wstring();
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
		wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
		return wstrTo;
	}
}