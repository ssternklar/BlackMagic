#include "..\include\LinkedList.h"

LinkedList::LinkedList(void* data)
{
	next = nullptr;
	this->data = data;
}

template<typename T>
T* LinkedList::GetAs()
{
	return static_cast<T>(data);
}