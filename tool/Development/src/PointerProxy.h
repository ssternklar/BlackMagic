#pragma once

template <typename T>
struct proxy_ctr
{
	T* ptr = nullptr;
	struct proxy_ptr
	{
		proxy_ptr() {}
		proxy_ptr(proxy_ctr<T>* c):ctr(c) {}
		T* ptr() { return ctr ? ctr->ptr : nullptr; }
		T* operator->() { return ctr ? ctr->ptr : nullptr; }
		T& operator *() { return *(ctr ? ctr->ptr : nullptr); }
		bool operator==(proxy_ptr& other) { return ctr == other.ctr; }
	private:
		proxy_ctr<T>* ctr = nullptr;
	};
	proxy_ptr handle;
	proxy_ctr() { handle = proxy_ptr(this); }
};

template<typename T>
class ProxyVector
{
public:
	ProxyVector();
	~ProxyVector();

	typename proxy_ctr<T>::proxy_ptr Track(T* ptr);
	void Relinquish(T* ptr);
	void Relinquish(T* ptr, size_t count);
	void Move(T* from, T* to);
	void Move(T* from, T* to, size_t count);
	typename proxy_ctr<T>::proxy_ptr Recover(T* ptr);

private:
	void Expand();

	template<typename T>
	struct proxy_index
	{
		size_t line = 0;
		size_t index = 0;
		proxy_ctr<T>* proxy = nullptr;
	};
	proxy_index<T> Find(T* ptr);

	const size_t lineWidth = 64 / sizeof(proxy_ctr<T>);
	size_t lineCount;
	proxy_ctr<T>** proxies;

	proxy_index<T> nextProxy;
};

template<typename T>
ProxyVector<T>::ProxyVector()
{
	proxies = nullptr;
	lineCount = 0;

	Expand();
	
	nextProxy.proxy = proxies[0];
}

template<typename T>
ProxyVector<T>::~ProxyVector()
{
	for (size_t i = 0; i < lineCount; ++i)
		delete proxies[i];
	delete[] proxies;
}

template<typename T>
void ProxyVector<T>::Expand()
{
	proxy_ctr<T>** newArr = new proxy_ctr<T>*[++lineCount];

	if (proxies)
	{
		memcpy_s(newArr, sizeof(proxy_ctr<T>**) * lineCount, proxies, sizeof(proxy_ctr<T>**) * (lineCount - 1));
		delete[] proxies;
	}

	newArr[lineCount - 1] = new proxy_ctr<T>[lineWidth];
	proxies = newArr;
}

template<typename T>
typename ProxyVector<T>::proxy_index<T> ProxyVector<T>::Find(T* ptr)
{
	proxy_index<T> index;

	for (size_t i = 0; i < lineCount; ++i)
		for (size_t j = 0; j < lineWidth; ++j)
			if (proxies[i][j].ptr == ptr)
			{
				index.line = i;
				index.index = j;
				index.proxy = &(proxies[i][j]);
				break;
			}

	return index;
}

template<typename T>
typename proxy_ctr<T>::proxy_ptr ProxyVector<T>::Track(T* ptr)
{
	proxy_ctr<T>* proxy = nextProxy.proxy;
	proxy->ptr = ptr;

	do
	{
		++nextProxy.index;
		nextProxy.line += nextProxy.index == lineWidth;
		nextProxy.index %= lineWidth;

		if (nextProxy.line >= lineCount)
			Expand();

		nextProxy.proxy = &(proxies[nextProxy.line][nextProxy.index]);
	} while (nextProxy.proxy->ptr);

	return proxy->handle;
}

template<typename T>
void ProxyVector<T>::Relinquish(T* ptr)
{
	proxy_index<T> proxy = Find(ptr);

	if (proxy.proxy)
	{
		proxy.proxy->ptr = nullptr;

		if (proxy.line * lineWidth + proxy.index < nextProxy.line * lineWidth + nextProxy.index)
			nextProxy = proxy;
	}
}

template<typename T>
void ProxyVector<T>::Relinquish(T* ptr, size_t count)
{
	proxy_index<T> proxy;

	for (size_t i = 0; i < count; ++i)
	{
		proxy = find(ptr + i);

		if (proxy.proxy)
		{
			proxy.proxy->ptr = nullptr;

			if (proxy.line * lineWidth + proxy.index < nextProxy.line * lineWidth + nextProxy.index)
				nextProxy = proxy;
		}
	}
}

template<typename T>
void ProxyVector<T>::Move(T* from, T* to)
{
	proxy_index<T> proxy = Find(from);

	if (proxy.proxy)
		proxy.proxy->ptr = to;
}

template<typename T>
void ProxyVector<T>::Move(T* from, T* to, size_t count)
{
	proxy_index<T> proxy;

	for (size_t i = 0; i < count; ++i)
	{
		proxy = Find(from + i);

		if (proxy.proxy)
			proxy.proxy->ptr = to + i;
	}
}

template<typename T>
typename proxy_ctr<T>::proxy_ptr ProxyVector<T>::Recover(T* ptr)
{
	return Find(ptr).proxy->handle;
}
