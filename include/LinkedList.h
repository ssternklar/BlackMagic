#pragma once

class LinkedList
{
public:
	LinkedList* next = nullptr;
	void* data;
	LinkedList(void* data);
	template<typename T>
	T* GetAs()
	{
		return static_cast<T*>(data);
	}
};

