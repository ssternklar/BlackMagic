#include "StringManip.h"

#include <sstream>

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
}