#pragma once

#include "PointerProxy.h"

template <class T>
class Singleton
{
public:
	static T& Instance()
	{
		static InstanceClass instance_object;
		return instance_object;
	}

protected:
	Singleton& operator=(const Singleton&) = delete;

private:
	virtual void SingletonHook() = 0;
	class InstanceClass : public T
	{
	private:
		void SingletonHook() {}
		InstanceClass& operator=(const InstanceClass&) = delete;
	};
};

template <class T, class S>
class Asset : public Singleton<S>
{
public:
	typedef typename proxy_ctr<T>::proxy_ptr Handle;

	size_t Size() { return size; };

protected:
	Asset();
	virtual ~Asset() { delete[] data; };

	virtual void Revoke(Handle handle);
	virtual Handle Get();

	size_t size;
	size_t growth;
	T* data;

private:
	size_t count;
	ProxyVector<T> proxy;
};

template <class T, class S>
Asset<T, S>::Asset()
{
	growth = 4;
	size = 0;
	count = growth;

	data = new T[count];
}

template <class T, class S>
typename Asset<T, S>::Handle Asset<T, S>::Get()
{
	if (size == count)
	{
		count += growth;
		T* newData = new T[count];

		memcpy_s(newData, sizeof(T) * count, data, sizeof(T) * (count - growth));
		proxy.Move(data, newData, count - growth);

		delete[] data;
		data = newData;
	}

	return proxy.Track(&data[size++]);
}

template <class T, class S>
void Asset<T, S>::Revoke(Handle handle)
{
	size_t index = handle.ptr() - data;
	proxy.Relinquish(handle.ptr());

	if (index != --size)
	{
		proxy.Move(data + size, data + index);
		data[index] = data[size];
	}
}
