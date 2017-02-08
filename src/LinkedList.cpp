#include "..\include\LinkedList.h"

LinkedList::LinkedList(void* data)
{
	next = nullptr;
	this->data = data;
}