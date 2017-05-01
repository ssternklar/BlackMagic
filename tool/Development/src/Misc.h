#pragma once

#include <string>

#include "Patterns.h"

struct Misc
{
	uint8_t dumb;
};

class MiscData : public ProxyHandler<Misc, MiscData>
{
public:
	Handle Get(std::string path);
	void Revoke(Handle entity);

	void Export(std::string path, Handle handle);
	Handle Load(std::string fullPath);
	const std::string root = "assets/misc/";
};